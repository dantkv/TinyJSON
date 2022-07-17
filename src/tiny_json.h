#ifndef TINY_JSON_H__
#define TINY_JSON_H__

/**
 * @brief JSON 基本类型枚举
 * 
 */
typedef enum {
    TINY_NUMBER,
    TINY_NULL,
    TINY_OBJECT,
    TINY_STRING,
    TINY_ARRAY,
    TINY_TRUE,
    TINY_FALSE
}tiny_type;

/**
 * @brief 解析结果类型
 * 
 */
enum {
    // 解析成功
    JSON_PARSE_OK,
    // JSON 只含有空白
    JSON_PARSE_EXPECT_VALUE,
    // 无效的值
    JSON_PARSE_INVALID_VALUE,
    // 在空白之后还有其他字符
    JSON_PARSE_ROOT_NOT_SINGULAR,
    //================ number ====================
    // 数字大小超出double最大范围
    JSON_PARSE_NUMBER_OUT_OF_RANGE,
    //================string====================
    // 字符串丢失 " 字符
    JOSN_PARSE_MISS_QUOTATION_MARK,
    // 转义字符无效
    JSON_PARSE_INVALID_STRING_ESCAPE,
    // 字符无效
    JSON_PARSE_INVALID_STRING_CHAR,
    // unicode编码错误
    JSON_PARSE_INVALID_UNICODE_HEX,
    // 不支持的 Unicode 编码
    JSON_PARSE_INVALID_UNICODE_SURROGATE,
    //================ array ====================
    // 数组解析丢失字符: `,` `]`
    JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    //================ object ====================
    // 丢失 key
    JSON_PARSE_MISS_KEY,
    // 丢失 value
    JSON_PARSE_MISS_COLON,
    // object 丢失符号
    JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET,

    // 
    JSON_STRINGIFY_OK,
    JSON_STRINGIFY_FALSE
};

typedef struct tiny_member tiny_member;

/**
 * @brief json节点
 * 注意字节对齐的问题
 */
struct tiny_value
{
    union 
    {
        // 当 type 为 TINY_NUMBER 时有效
        double number;
        struct
        {
            size_t str_len;
            char *str;
        };
        struct 
        {
            // 数组的指针 堆上连续的空间存储
            tiny_value * arr_ptr;
            // 数组存储元素个数
            size_t arr_size;
        };
        struct 
        {
            tiny_member * member_ptr;
            size_t member_size;
        };
    };
    
    tiny_type value_type;
};

struct tiny_member
{
    char *key;  // object key
    size_t len; // key length
    tiny_value value;
};

void tiny_init(tiny_value *value);

void tiny_free(tiny_value *value);

tiny_type get_type(const tiny_value *value);

/**
 * @brief 解析入口
 * @details value = null / false / true
 * @param value 
 * @param json 
 * @return LEPT_PARSE_ROOT_NOT_SINGULAR 当解析完一个值后有空格还有其他值
 */
int tiny_parse(const char *json, tiny_value *value);

// ---------------- bool ----------------

int get_bool(const tiny_value *value);

void set_bool(tiny_value *value, int val);

// ---------------- string ----------------

size_t get_string_len(const tiny_value *value);

const char *get_string(const tiny_value *value);

void set_string(tiny_value *value,const char *str, int len);


// ---------------- number ----------------

double get_number(const tiny_value *value);

void set_number(tiny_value * value, double num);

// ---------------- array ----------------

size_t get_array_size(const tiny_value *value);

tiny_value * get_array(const tiny_value *value);

/**
 * @brief 获取数组index下标的节点
 * 
 * @return tiny_value* 
 */
tiny_value * get_array_element(const tiny_value *value, size_t index);

// ---------------- object ----------------
size_t get_object_size(const tiny_value *value);

const char * get_object_key(const tiny_value *value, size_t index);

size_t get_object_key_length(const tiny_value *value, size_t index);

tiny_value *get_object(const tiny_value *value, size_t index);

// ---------------- stringify ----------------

int tiny_stringify(const tiny_value *value, char ** json, size_t *length);

#endif