#include<cassert>
#include<cstring>
#include<iostream>
#include<cstdlib>
#include<cmath>

#include "tiny_json.h"

typedef struct
{
    const char *json;
    char *stack; // 存储任意长度字符的栈指针
    size_t size; // 当前堆容量
    size_t top; // 栈顶位置
}tiny_context;

#define DEFAULT_SIZE 16
#define DEFAULT_STRINGIFY_SIZE 128
#define DEFAULT_NUMBER_MAX_LENGTH 32

void tiny_init(tiny_value *value){
    value->value_type = TINY_NULL;
}

void tiny_free(tiny_value *value){
    assert(value != nullptr);
    switch (value->value_type)
    {
    case TINY_STRING:
        free(value->str);
        value->str = nullptr;
        break;
    case TINY_ARRAY:
        // 由于元素里面可能有字符串，需要对每个元素分别释放，再释放数组本身
        for (size_t i = 0; i < value->arr_size; i++){
            tiny_free(&value->arr_ptr[i]);
        }
        free(value->arr_ptr);
        value->arr_ptr = nullptr;
        break;
    case TINY_OBJECT:
        for (size_t i = 0; i < value->member_size; i++){
            free(value->member_ptr[i].key);
            value->member_ptr[i].key = nullptr;
            tiny_free(&value->member_ptr[i].value);
        }
        free(value->member_ptr);
        value->member_ptr = nullptr;
        break;
    default:
        break;
    }
    value->value_type = TINY_NULL;
}

tiny_type get_type(const tiny_value *value){
    assert(value != nullptr);
    return value->value_type;
}

static int tiny_parse_value(tiny_context *context, tiny_value *value);

static void tiny_parse_whitespace(tiny_context *context){
    const char *p = context->json;
    while(*p == ' ' || *p == '\r' || *p == '\n' || *p == '\t'){
        p++;
    }
    context->json = p;
}

int tiny_parse(const char *json, tiny_value *value){
    tiny_context context;
    assert(json != nullptr);
    context.json = json;
    context.stack = nullptr;
    context.top = context.size = 0;
    tiny_init(value);

    tiny_parse_whitespace(&context);
    int result = tiny_parse_value(&context, value);
    if(JSON_PARSE_OK == result){
        tiny_parse_whitespace(&context);
        if(context.json[0] != '\0'){
            // 如果解析失败，类型为 null
            value->value_type = TINY_NULL;
            return JSON_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(context.top == 0);
    free(context.stack);
    return result;
}

/**
 * @brief 
 *  
 * @return void* size大小的数据在栈中的地址
 */
static void *context_push(tiny_context *c, size_t size){
    assert(c != nullptr);
    void *ret = nullptr;
    if (c->top+size >= c->size){
        if (c->size == 0){
            c->size = DEFAULT_SIZE;
        }
        while(c->top+size >= c->size){
            // 每次扩大到原来2倍
            c->size = c->size<<1;
        }
        // 重新分配内存并返回新内存地址
        c->stack = (char *)realloc(c->stack, c->size);
    }
    
    // ret 记录下当前分配地址开始位置
    ret = c->top+c->stack;

    c->top += size;
    return ret;
}

inline void PUTC(tiny_context *ctx, char c){
    *(char *)context_push(ctx, sizeof(char)) = c;
}

/**
 * @brief 返回弹出的字符串地址
 * @details 没有清除字符，返回后需要使用
 * @return void* 
 */
static void * context_pop(tiny_context *c, size_t size){
    assert(c != nullptr);
    assert(c->top >= size);
    c->top = c->top-size;
    return c->stack + c->top;
}

// ---------------- bool ----------------

static int tiny_parse_literal(tiny_context *ctx, tiny_value *val,const char * str, tiny_type t){
    int i = 0;
    for (i = 0; str[i]; i++){
        if (!ctx->json[i] || ctx->json[i] != str[i]){
            return JSON_PARSE_INVALID_VALUE;
        }
    }
    ctx->json += i;
    val->value_type = t;
    return JSON_PARSE_OK;
}

int get_bool(const tiny_value *value){
    assert(value!= nullptr);
    assert(value->value_type != TINY_FALSE || value->value_type != TINY_TRUE);
    return value->value_type ;
}

void set_bool(tiny_value *value, int val){
    assert(value!=nullptr);
    // 初始化tiny_value
    tiny_free(value);
    value->value_type = val ? TINY_TRUE : TINY_FALSE;
}

// ---------------- string ----------------

#define STRING_ERROR(ctx, top, err) do{\
        ctx->top = top;\
        return err;\
    }while(0);

/**
 * @brief 按长度比较字符串是否相同
 * 
 * @return 1 相同 0 不同
 *  
 */
int strcmp_s(const char *s1, const char *s2, size_t len){
    size_t i = 0;
    while(i < len && s1[i] && s1[i] == s2[i]){
        i++;
    }
    return i == len;
}

/**
 * @brief 解析 4 位十六进数字，存储为码点 u。
 * 将u后面四位十六进制的字符转换为 u int
 * @return 成功时返回解析后的文本指针，失败返回 NULL
 */
static char * tiny_parse_hex4(char *p, unsigned int *u){
    *u = 0;
    for (int i = 0; i < 4; i++) {
        char ch = *p++;
        *u <<= 4;
        // 使用或来进行按位加法
        if      (ch >= '0' && ch <= '9')  *u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F')  *u |= ch - 'A' + 10;
        else if (ch >= 'a' && ch <= 'f')  *u |= ch - 'a' + 10;
        else return NULL;
    }
    return p;
}

/**
 * @brief 将编码为 uxxxx 的Unicode编码解码为多个字符编码组合 如 \xF0\x9D\x84\x9E 
 */
static void tiny_encode_utf8(tiny_context *c, unsigned int u){
    if (u <= 0x007f){
        /* 码点 7位
            这里 u & 0x7f 因为编译器可能会报警告数据截断丢失的风险
            实际上这里已经判断了大小在255范围内，一般编译器会优化掉这个操作，不会影响性能。*/
        PUTC(c, u & 0x7f);
    }else if( u <= 0x07ff){
        /* 码点11位 110xxxxx 10xxxxxx */
        PUTC(c, 0xC0 |((u>>6) & 0x1f)); // 右移6位 取后5位
        PUTC(c, 0x80 | (u & 0x3f)); // u & 0x3f 取最低6位数字 然后将最高2位置为 10
    }else if (u <= 0xffff){
        /* 码点16位 1110xxxx 10xxxxxx 10xxxxxx */
        PUTC(c, 0xE0 | ((u>>12) & 0xff));
        PUTC(c, 0x80 | ((u>>6) & 0x3f)); // 右移6位，然后取低6位 相当于取中间6位
        PUTC(c, 0x80 | (u & 0x3f)); //  
    }else if (u <= 0x10ffff){
        /*
        码点24位 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */ 
        PUTC(c, 0xF0 | ((u>>18) & 0x03));
        PUTC(c, 0x80 | ((u>>12) & 0x3f));
        PUTC(c, 0x80 | ((u>>6) & 0x3f));
        PUTC(c, 0x80 | (u & 0x3f));
    }
}

static int tiny_parse_string_raw(tiny_context *ctx, char ** str, size_t * len){
    /*
    从字符 \" 开始到字符 \"
    计算中间的字符长度，然后放入到context->stack
    完成一次字符计算后，将其copy到value->str,并将stack还原

    为什么需要使用字符串缓冲区来保存？
    
    因为有部分字符是转义字符，需要进行二次处理，例如Unicode编码，转义后可能长度比之前长，不能直接替换到原来位置。
    =================
    优化方案：
    可以将无需转义的字符串直接复制到context->str
    遇到转义字符转义后单独复制过去

    循环上面步骤
    */

   // 备份栈顶位置
    size_t top = ctx->top;
    const char *p = ctx->json;
    if (p[0] != '\"'){
        return JOSN_PARSE_MISS_QUOTATION_MARK;
    }
    p++;
    char c;
    unsigned int u_high, u_low;
    while(true){
        c = *p++;
        switch(c){
            case '\"':
                // 到达字符串末尾，弹出栈字符并保存
                *len = ctx->top-top;
                *str =  (char *)context_pop(ctx, *len);
                ctx->json = p;
                return JSON_PARSE_OK;
            case '\0':
                // 字符串中途结束 返回错误信息
                STRING_ERROR(ctx, top, JOSN_PARSE_MISS_QUOTATION_MARK);
            case '\\':
                // 转义字符占原字符串2位，这里p再移一位，跳到下个待处理字符
                switch (*p++)
                {
                case '\"':  PUTC(ctx, '\"');  break;
                case '\\':  PUTC(ctx, '\\');  break;
                case '/':  PUTC(ctx, '/');  break;
                case 'b':  PUTC(ctx, '\b');  break;
                case 'f':  PUTC(ctx, '\f');  break;
                case 'n':  PUTC(ctx, '\n');  break;
                case 'r':  PUTC(ctx, '\r');  break;
                case 't':  PUTC(ctx, '\t');  break;
                case 'u':
                    /*
                    解析UTF-8编码
                    \u20AC 首先将后面16进制编码使用 unsigned int 存储，然后根据码点表将其拆分成多个16进制编码，然后存储。
                    */
                    if (!(p = tiny_parse_hex4(const_cast<char *>(p), &u_high))){
                        STRING_ERROR(ctx, top, JSON_PARSE_INVALID_UNICODE_HEX);
                    }
                    /*
                    U+0000 至 U+FFFF 这组 Unicode 字符称为基本多文种平面（basic multilingual plane, BMP）
                    还有另外 16 个平面。那么 BMP 以外的字符，JSON 会使用代理对（surrogate pair）表示 \uXXXX\uYYYY。
                    在 BMP 中，保留了 2048 个代理码点。
                    如果第一个码点是 U+D800 至 U+DBFF，我们便知道它的代码对的高代理项（high surrogate），
                    之后应该伴随一个 U+DC00 至 U+DFFF 的低代理项（low surrogate）。
                    然后，我们用下列公式把代理对 (H, L) 变换成真实的码点：
                    codepoint = 0x10000 + (H − 0xD800) × 0x400 + (L − 0xDC00)
                    */
                    if (u_high >= 0xD800 && u_high <= 0xDBFF) { /* surrogate pair */
                        if (*p++ != '\\')
                            STRING_ERROR(ctx, top, JSON_PARSE_INVALID_UNICODE_SURROGATE);
                        if (*p++ != 'u')
                            STRING_ERROR(ctx, top, JSON_PARSE_INVALID_UNICODE_SURROGATE);
                        if (!(p = tiny_parse_hex4(const_cast<char *>(p), &u_low)))
                            STRING_ERROR(ctx, top, JSON_PARSE_INVALID_UNICODE_HEX);
                        if (u_low < 0xDC00 || u_low > 0xDFFF)
                            STRING_ERROR(ctx, top, JSON_PARSE_INVALID_UNICODE_SURROGATE);
                        u_high = (((u_high - 0xD800) << 10) | (u_low - 0xDC00)) + 0x10000;
                    }
                    tiny_encode_utf8(ctx, u_high);
                    break;
                default:
                    STRING_ERROR(ctx, top, JSON_PARSE_INVALID_STRING_ESCAPE);
                    break;
                }
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20){
                    STRING_ERROR(ctx, top, JSON_PARSE_INVALID_STRING_CHAR);
                }
                // 每次拷贝一个char到缓冲区
                PUTC(ctx, c);
        }
    }
    
    // 如果没有在循环中退出，那么出现了未知错误
    return JOSN_PARSE_MISS_QUOTATION_MARK;
}

static int tiny_parse_string(tiny_context *ctx, tiny_value *value){
    int result = -1;
    char *str;
    size_t len;
    result = tiny_parse_string_raw(ctx, &str, &len);
    if (result == JSON_PARSE_OK){
        set_string(value, str, len); 
    }
    return result;
}

size_t get_string_len(const tiny_value *value){
    assert(value != nullptr && value->value_type == TINY_STRING);
    return value->str_len;
}

const char *get_string(const tiny_value *value){
    assert(value != nullptr);
    if(value->value_type == TINY_STRING){
        return value->str;
    }
    return nullptr;
}

void set_string(tiny_value *value, const char *str, int len){
    assert(value != nullptr && (str != nullptr || len == 0));
    tiny_free(value);
    value->str = (char *)malloc(len+1);
    memcpy(value->str, str, len);
    value->str_len = len;
    value->str[len] = '\0';
    value->value_type = TINY_STRING;
}

// ---------------- number ----------------

inline bool ISDIGIT(int number){
    return number>='0' && number <= '9';
}

inline bool ISDIGIT_1TO9(int number){
    return number >= '1' && number <= '9';
}

static int tiny_parse_number(tiny_context *context, tiny_value *value){
    // 校验格式正确性
    assert(context->json!=nullptr);
    char *ptr = const_cast<char *>(context->json);
    if (*ptr == '-'){
        ptr++;
    }
    if (*ptr == '0'){
        ptr++;
    }else{
        if (!ISDIGIT_1TO9(*ptr)){
            return JSON_PARSE_INVALID_VALUE;
        }
        ptr++;
        for (; ISDIGIT(*ptr); ptr++);
    }
    if (*ptr == '.'){
        ptr++;
        if (!ISDIGIT(*ptr)){
            return JSON_PARSE_INVALID_VALUE;
        }
        ptr++;
        for(; ISDIGIT(*ptr); ptr++);
    }

    // E e + digit
    if (*ptr== 'e' || *ptr=='E'){
        ptr++;
        if (*ptr == '+' || *ptr == '-'){
            ptr++;
        }
        if (!ISDIGIT(*ptr)){
            return JSON_PARSE_INVALID_VALUE;
        }
        for(; ISDIGIT(*ptr); ptr++);
    }
    
    // 调用库函数进行转换
    double d = strtod(context->json, nullptr);
    if (d == INFINITY || d == -INFINITY){
        return JSON_PARSE_NUMBER_OUT_OF_RANGE;
    }
    
    context->json = ptr;
    value->value_type = TINY_NUMBER;
    value->number = d;
    return JSON_PARSE_OK;
}

double get_number(const tiny_value *value){
    assert(value != nullptr && value->value_type == TINY_NUMBER);
    return value->number;
}

void set_number(tiny_value * value, double num){
    assert(value != nullptr);
    tiny_free(value);
    value->value_type = TINY_NUMBER;
    value->number = num;
}

// ---------------- array ----------------

/**
 * @brief 解析数组
 * 
 * @param ctx 
 * @param value 
 * @return int 
 */
static int tiny_parse_array(tiny_context * ctx, tiny_value * value){
    assert(*ctx->json == '[');
    ctx->json++;
    tiny_parse_whitespace(ctx);
    size_t size = 0;
    int result = -1;
    if (*ctx->json == ']'){
        value->value_type = TINY_ARRAY;
        value->arr_size = 0;
        value->arr_ptr = nullptr;
        ctx->json++;
        return JSON_PARSE_OK;
    }
    while(true){
        tiny_value temp_tiny_value;
        tiny_init(&temp_tiny_value);
        tiny_parse_whitespace(ctx);
        result = tiny_parse_value(ctx, &temp_tiny_value);
        if (result != JSON_PARSE_OK){
            break;
        }
        
        memcpy(context_push(ctx, sizeof(tiny_value)), &temp_tiny_value, sizeof(tiny_value));
        size ++;
        tiny_parse_whitespace(ctx);
        if (*ctx->json == ','){
            ctx->json++;
        }else if (*ctx->json == ']'){
            ctx->json++;
            value->value_type = TINY_ARRAY;
            value->arr_size = size;
            size *= sizeof(tiny_value);
            value->arr_ptr = (tiny_value*)malloc(size);
            memcpy(value->arr_ptr, context_pop(ctx, size), size);
            return JSON_PARSE_OK;
        }else{
            result = JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
    // 对出错时已入栈的元素依次释放内存
    for (size_t i = 0; i < size; i++){
        tiny_free((tiny_value *)context_pop(ctx, sizeof(tiny_value)));
    }
    return result;
}

size_t get_array_size(const tiny_value *value){
    assert(value != nullptr && value->value_type == TINY_ARRAY);
    return value->arr_size;
}

tiny_value * get_array(const tiny_value *value){
    assert(value != nullptr && value->value_type == TINY_ARRAY);
    return value->arr_ptr;
}

tiny_value * get_array_element(const tiny_value *value, size_t index){
    assert(value != nullptr && value->value_type == TINY_ARRAY);
    assert(value->arr_size > index);
    return &value->arr_ptr[index];
}

// ---------------- object ----------------

size_t get_object_size(const tiny_value *value){
    assert(value != nullptr && value->value_type == TINY_OBJECT);
    return value->member_size;
}

const char * get_object_key(const tiny_value *value, size_t index){
    assert(value != nullptr && value->value_type == TINY_OBJECT);
    assert(value->member_size > index);
    return value->member_ptr[index].key;
}

size_t get_object_key_length(const tiny_value *value, size_t index){
    assert(value != nullptr && value->value_type == TINY_OBJECT);
    assert(value->member_size > index);
    return value->member_ptr[index].len;
}

tiny_value *get_object(const tiny_value *value, size_t index){
    assert(value != nullptr && value->value_type == TINY_OBJECT);
    assert(value->member_size > index);
    return &value->member_ptr[index].value;
}

static int tiny_parse_object(tiny_context *ctx, tiny_value *value){
    assert(*ctx->json == '{');
    ctx->json++;
    tiny_parse_whitespace(ctx);
    size_t size = 0;
    int result = -1;
    if (*ctx->json == '}'){
        value->value_type = TINY_OBJECT;
        value->member_size = 0;
        value->member_ptr = nullptr;
        ctx->json++;
        return JSON_PARSE_OK;
    }
    tiny_member temp_tiny_member;
    temp_tiny_member.key = nullptr;
    while(true){
        if (ctx->json[0] != '\"'){
            result = JSON_PARSE_MISS_KEY;
            break;
        }
        tiny_init(&temp_tiny_member.value);
        // 解析类名
        char * name;
        result = tiny_parse_string_raw(ctx, &name, &temp_tiny_member.len);
        if (result != JSON_PARSE_OK){
            break;
        }
        temp_tiny_member.key = (char *)malloc(temp_tiny_member.len+1);
        memcpy(temp_tiny_member.key, name, temp_tiny_member.len);
        temp_tiny_member.key[temp_tiny_member.len] = '\0';

        tiny_parse_whitespace(ctx);
        if (*ctx->json != ':'){
            result = JSON_PARSE_MISS_COLON;
            break;
        }
        ctx->json++;
        tiny_parse_whitespace(ctx);

        // 解析类
        result = tiny_parse_value(ctx, &temp_tiny_member.value);
        if (result != JSON_PARSE_OK){
            result = JSON_PARSE_MISS_COLON;
            break;
        }
        
        memcpy(context_push(ctx, sizeof(tiny_member)), &temp_tiny_member, sizeof(tiny_member));
        size++;
        temp_tiny_member.key = nullptr;
        tiny_parse_whitespace(ctx);
        if (*ctx->json == ','){
            ctx->json++;
            tiny_parse_whitespace(ctx);
        }else if (*ctx->json == '}'){
            ctx->json++;
            value->value_type = TINY_OBJECT;
            value->member_size = size;
            size *= sizeof(tiny_member);
            value->member_ptr = (tiny_member*)malloc(size);
            memcpy(value->member_ptr, context_pop(ctx, size), size);
            return JSON_PARSE_OK;
        }else{
            result = JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    // 如果key解析一半出错，那么释放已使用的栈空间
    free(temp_tiny_member.key);
    // 对出错时已入栈的元素依次释放内存
    for (size_t i = 0; i < size; i++){
        tiny_member * tmp = (tiny_member *)context_pop(ctx, sizeof(tiny_member));
        free(tmp->key);
        tmp->key = nullptr;
        tiny_free(&tmp->value);
    }
    return result;
}

// ========================================================

static int tiny_parse_value(tiny_context *context, tiny_value *value){
    if  ( ISDIGIT( context->json[0]) || context->json[0] == '-'){
        return tiny_parse_number(context, value);
    }
    switch (context->json[0])
    {
    case 'n':
        return tiny_parse_literal(context, value, "null", TINY_NULL);
    case 'f':
        return tiny_parse_literal(context, value, "false", TINY_FALSE);
    case 't':
        return tiny_parse_literal(context, value, "true", TINY_TRUE);
    case '\"':
        return tiny_parse_string(context, value);
    case '[':
        return tiny_parse_array(context, value);
    case '{':
        return tiny_parse_object(context, value);
    case '\0':
        // 去掉空格后字符串结束了，只包含空格
        return JSON_PARSE_EXPECT_VALUE;
    default:
        return JSON_PARSE_INVALID_VALUE;
    }
}

// ---------------- stringify ----------------

static int tiny_stringify_value(tiny_context *ctx, const tiny_value *value);

static int tiny_stringify_number(tiny_context *ctx, const tiny_value *value){
    char *num = (char *)context_push(ctx, DEFAULT_NUMBER_MAX_LENGTH);
    int len = sprintf(num, "%.17g", value->number);
    ctx->top -= DEFAULT_NUMBER_MAX_LENGTH-len;
    return JSON_STRINGIFY_OK;
}

static int tiny_stringify_string(tiny_context *ctx, const tiny_value *value){
    /**
     * 为避免频繁调用PUTC，可以直接操作栈指针进行字符存储
     */
    
    // unicode编码占6字节，加前后双引号2字节

    size_t size = value->str_len*6+2;
    char *ptr = (char *)context_push(ctx, size);
    char *head = ptr;
    *ptr++ = '\"';
    for(size_t i = 0; i < value->str_len; i++){
        char c = value->str[i];
        switch (c) {
            case '\\': 
                *ptr++ = '\\';
                *ptr++ = '\\';
                break;
            case '\"':
                *ptr++ =  '\\';
                *ptr++ =  '\"';
                break;
            case '/':
                *ptr++ = '\\';
                *ptr++ = '/';
                break;
            case '\b':
                *ptr++ = '\\';
                *ptr++ = 'b';
                break;
            case '\f':
                *ptr++ = '\\';
                *ptr++ =  'f';
                break;
            case '\n':
                *ptr++ = '\\';
                *ptr++ = 'n';
                break;
            case '\r':
                *ptr++ = '\\';
                *ptr++ = 'r';
                break;
            case '\t':
                *ptr++ = '\\';
                *ptr++ = 't';
                break;
            default:
                *ptr++ = c;
                break;
        }
    }
    *ptr++ = '\"';
    ctx->top -= size - (ptr-head);
    return JSON_STRINGIFY_OK;
}

static int tiny_stringify_array(tiny_context *ctx, const tiny_value *value){
    PUTC(ctx, '[');
    for (size_t i = 0; i < value->arr_size; i++){
        int result = tiny_stringify_value(ctx, &value->arr_ptr[i]);
        if (result == JSON_STRINGIFY_OK && i < value->arr_size-1){
            PUTC(ctx, ',');
        }
    }
    PUTC(ctx, ']');
    return JSON_STRINGIFY_OK;
}

static int tiny_stringify_object(tiny_context *ctx, const tiny_value *value){
    PUTC(ctx, '{');
    for (size_t i = 0; i < value->member_size; i++){
        PUTC(ctx, '\"');
        char * key = (char *)context_push(ctx, value->member_ptr[i].len);
        memcpy(key, value->member_ptr[i].key, value->member_ptr[i].len);
        PUTC(ctx, '\"');
        PUTC(ctx, ':');
        int result = tiny_stringify_value(ctx, &value->member_ptr[i].value);
        if (result == JSON_STRINGIFY_OK && i < value->member_size-1){
            PUTC(ctx, ',');
        }
    }
    PUTC(ctx, '}');
    return JSON_STRINGIFY_OK;
}

static int tiny_stringify_value(tiny_context *ctx, const tiny_value *value){
    switch (value->value_type)
    {
    case TINY_FALSE:
        memcpy(context_push(ctx, 5), "false", 5);
        break;
    case TINY_TRUE:
        memcpy(context_push(ctx, 4), "true", 4);
        break;
    case TINY_NULL:
        memcpy(context_push(ctx, 4), "null", 4);
        break;
    case TINY_NUMBER:
        return tiny_stringify_number(ctx, value);
    case TINY_STRING:
        return tiny_stringify_string(ctx, value);
    case TINY_ARRAY:
        return tiny_stringify_array(ctx, value);
    case TINY_OBJECT:
        return tiny_stringify_object(ctx, value);
    default:    return JSON_STRINGIFY_FALSE;
    }
    return JSON_STRINGIFY_OK;
}

int tiny_stringify(const tiny_value *value, char ** json, size_t *length){
    assert(value != nullptr);
    assert(json != nullptr);
    tiny_context ctx;
    ctx.size = DEFAULT_STRINGIFY_SIZE;
    ctx.stack = (char *)malloc(ctx.size);
    ctx.json = nullptr;
    ctx.top = 0;
    int result = tiny_stringify_value(&ctx, value);
    if (JSON_STRINGIFY_OK != result){
        free(ctx.stack);
        ctx.stack = nullptr;
        *json = nullptr;
        ctx.size = 0;
        return result;
    }
    if (length){
        *length = ctx.top;
    }
    *(char *)context_push(&ctx, 1) = '\0';
    *json = ctx.stack;
    return JSON_STRINGIFY_OK;
}




