#include <stdio.h>
#include <assert.h>
#include <fixedptc.h>

// 辅助函数：打印测试结果
static void test_result(const char *name, int passed) {
    printf("[%s] %s\n", passed ? "PASS" : "FAIL", name);
}

// 辅助函数：比较两个 fixedpt 值（允许小的误差）
static int fixedpt_equal(fixedpt a, fixedpt b) {
    fixedpt diff = fixedpt_abs(fixedpt_sub(a, b));
    // 允许误差在 1 个单位以内（即 1/256）
    return diff <= 1;
}

// 测试 fixedpt_muli：定点数乘以整数
void test_fixedpt_muli() {
    printf("\n=== Testing fixedpt_muli ===\n");
    
    // 测试：1.5 * 2 = 3.0
    fixedpt a = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    fixedpt result = fixedpt_muli(a, 2);
    fixedpt expected = fixedpt_fromint(3); // 3.0
    test_result("1.5 * 2 = 3.0", fixedpt_equal(result, expected));
    
    // 测试：2.5 * 4 = 10.0
    a = fixedpt_fromint(2) + FIXEDPT_ONE_HALF; // 2.5
    result = fixedpt_muli(a, 4);
    expected = fixedpt_fromint(10); // 10.0
    test_result("2.5 * 4 = 10.0", fixedpt_equal(result, expected));
    
    // 测试：0.5 * 3 = 1.5
    a = FIXEDPT_ONE_HALF; // 0.5
    result = fixedpt_muli(a, 3);
    expected = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    test_result("0.5 * 3 = 1.5", fixedpt_equal(result, expected));
    
    // 测试：负数：-1.5 * 2 = -3.0
    a = -(fixedpt_fromint(1) + FIXEDPT_ONE_HALF); // -1.5
    result = fixedpt_muli(a, 2);
    expected = -fixedpt_fromint(3); // -3.0
    test_result("-1.5 * 2 = -3.0", fixedpt_equal(result, expected));
    
    // 测试：乘以 0
    a = fixedpt_fromint(5);
    result = fixedpt_muli(a, 0);
    expected = 0;
    test_result("5 * 0 = 0", fixedpt_equal(result, expected));
    
    // 测试：乘以 1
    a = fixedpt_fromint(7);
    result = fixedpt_muli(a, 1);
    expected = fixedpt_fromint(7);
    test_result("7 * 1 = 7", fixedpt_equal(result, expected));
    
    // === 新增测试用例：更多小数结果的整数乘法 ===
    
    // 测试：1.5 * 3 = 4.5
    a = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    result = fixedpt_muli(a, 3);
    expected = fixedpt_fromint(4) + FIXEDPT_ONE_HALF; // 4.5
    test_result("1.5 * 3 = 4.5", fixedpt_equal(result, expected));
    
    // 测试：2.5 * 3 = 7.5
    a = fixedpt_fromint(2) + FIXEDPT_ONE_HALF; // 2.5
    result = fixedpt_muli(a, 3);
    expected = fixedpt_fromint(7) + FIXEDPT_ONE_HALF; // 7.5
    test_result("2.5 * 3 = 7.5", fixedpt_equal(result, expected));
    
    // 测试：0.5 * 5 = 2.5
    a = FIXEDPT_ONE_HALF; // 0.5
    result = fixedpt_muli(a, 5);
    expected = fixedpt_fromint(2) + FIXEDPT_ONE_HALF; // 2.5
    test_result("0.5 * 5 = 2.5", fixedpt_equal(result, expected));
    
    // 测试：0.25 * 7 = 1.75
    a = FIXEDPT_ONE >> 2; // 0.25
    result = fixedpt_muli(a, 7);
    expected = fixedpt_fromint(1) + (FIXEDPT_ONE >> 1) + (FIXEDPT_ONE >> 2); // 1.75
    test_result("0.25 * 7 = 1.75", fixedpt_equal(result, expected));
    
    // 测试：负数：-1.5 * 2 = -3.0
    a = -(fixedpt_fromint(1) + FIXEDPT_ONE_HALF); // -1.5
    result = fixedpt_muli(a, 2);
    expected = -fixedpt_fromint(3); // -3.0
    test_result("-1.5 * 2 = -3.0", fixedpt_equal(result, expected));
    
    // 测试：负数小数结果：-1.5 * 3 = -4.5
    a = -(fixedpt_fromint(1) + FIXEDPT_ONE_HALF); // -1.5
    result = fixedpt_muli(a, 3);
    expected = -(fixedpt_fromint(4) + FIXEDPT_ONE_HALF); // -4.5
    test_result("-1.5 * 3 = -4.5", fixedpt_equal(result, expected));
}

// 测试 fixedpt_divi：定点数除以整数
void test_fixedpt_divi() {
    printf("\n=== Testing fixedpt_divi ===\n");
    
    // 测试：6.0 / 2 = 3.0
    fixedpt a = fixedpt_fromint(6); // 6.0
    fixedpt result = fixedpt_divi(a, 2);
    fixedpt expected = fixedpt_fromint(3); // 3.0
    test_result("6.0 / 2 = 3.0", fixedpt_equal(result, expected));
    
    // 测试：10.0 / 4 = 2.5
    a = fixedpt_fromint(10); // 10.0
    result = fixedpt_divi(a, 4);
    expected = fixedpt_fromint(2) + FIXEDPT_ONE_HALF; // 2.5
    test_result("10.0 / 4 = 2.5", fixedpt_equal(result, expected));
    
    // 测试：1.5 / 3 = 0.5
    a = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    result = fixedpt_divi(a, 3);
    expected = FIXEDPT_ONE_HALF; // 0.5
    test_result("1.5 / 3 = 0.5", fixedpt_equal(result, expected));
    
    // 测试：负数：-6.0 / 2 = -3.0
    a = -fixedpt_fromint(6); // -6.0
    result = fixedpt_divi(a, 2);
    expected = -fixedpt_fromint(3); // -3.0
    test_result("-6.0 / 2 = -3.0", fixedpt_equal(result, expected));
    
    // 测试：除以 1
    a = fixedpt_fromint(5);
    result = fixedpt_divi(a, 1);
    expected = fixedpt_fromint(5);
    test_result("5 / 1 = 5", fixedpt_equal(result, expected));
    
    // === 新增测试用例：更多小数结果的整数除法 ===
    
    // 测试：3.0 / 2 = 1.5
    a = fixedpt_fromint(3); // 3.0
    result = fixedpt_divi(a, 2);
    expected = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    test_result("3.0 / 2 = 1.5", fixedpt_equal(result, expected));
    
    // 测试：7.0 / 4 = 1.75
    a = fixedpt_fromint(7); // 7.0
    result = fixedpt_divi(a, 4);
    expected = fixedpt_fromint(1) + (FIXEDPT_ONE >> 1) + (FIXEDPT_ONE >> 2); // 1.75
    test_result("7.0 / 4 = 1.75", fixedpt_equal(result, expected));
    
    // 测试：9.0 / 4 = 2.25
    a = fixedpt_fromint(9); // 9.0
    result = fixedpt_divi(a, 4);
    expected = fixedpt_fromint(2) + (FIXEDPT_ONE >> 2); // 2.25
    test_result("9.0 / 4 = 2.25", fixedpt_equal(result, expected));
    
    // 测试：1.0 / 3 ≈ 0.333...
    a = fixedpt_fromint(1); // 1.0
    result = fixedpt_divi(a, 3);
    expected = FIXEDPT_ONE / 3; // 0.333... ≈ 85
    test_result("1.0 / 3 ≈ 0.333", fixedpt_equal(result, expected));
    
    // 测试：1.0 / 4 = 0.25
    a = fixedpt_fromint(1); // 1.0
    result = fixedpt_divi(a, 4);
    expected = FIXEDPT_ONE >> 2; // 0.25 = 64
    test_result("1.0 / 4 = 0.25", fixedpt_equal(result, expected));
    
    // 测试：3.0 / 4 = 0.75
    a = fixedpt_fromint(3); // 3.0
    result = fixedpt_divi(a, 4);
    expected = (FIXEDPT_ONE >> 1) + (FIXEDPT_ONE >> 2); // 0.75 = 192
    test_result("3.0 / 4 = 0.75", fixedpt_equal(result, expected));
    
    // 测试：负数小数结果：-5.0 / 2 = -2.5
    a = -fixedpt_fromint(5); // -5.0
    result = fixedpt_divi(a, 2);
    expected = -(fixedpt_fromint(2) + FIXEDPT_ONE_HALF); // -2.5
    test_result("-5.0 / 2 = -2.5", fixedpt_equal(result, expected));
    
    // 测试：负数小数结果：-3.0 / 4 = -0.75
    a = -fixedpt_fromint(3); // -3.0
    result = fixedpt_divi(a, 4);
    expected = -((FIXEDPT_ONE >> 1) + (FIXEDPT_ONE >> 2)); // -0.75
    test_result("-3.0 / 4 = -0.75", fixedpt_equal(result, expected));
    
    // 测试：小数除以整数：2.5 / 5 = 0.5
    a = fixedpt_fromint(2) + FIXEDPT_ONE_HALF; // 2.5
    result = fixedpt_divi(a, 5);
    expected = FIXEDPT_ONE_HALF; // 0.5
    test_result("2.5 / 5 = 0.5", fixedpt_equal(result, expected));
    
    // 测试：小数除以整数：1.25 / 5 = 0.25
    a = fixedpt_fromint(1) + (FIXEDPT_ONE >> 2); // 1.25
    result = fixedpt_divi(a, 5);
    expected = FIXEDPT_ONE >> 2; // 0.25
    test_result("1.25 / 5 = 0.25", fixedpt_equal(result, expected));
}

// 测试 fixedpt_mul：两个定点数相乘
void test_fixedpt_mul() {
    printf("\n=== Testing fixedpt_mul ===\n");
    
    // 测试：2.0 * 1.5 = 3.0
    fixedpt a = fixedpt_fromint(2); // 2.0
    fixedpt b = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    fixedpt result = fixedpt_mul(a, b);
    fixedpt expected = fixedpt_fromint(3); // 3.0
    test_result("2.0 * 1.5 = 3.0", fixedpt_equal(result, expected));
    
    // 测试：2.5 * 2.0 = 5.0
    a = fixedpt_fromint(2) + FIXEDPT_ONE_HALF; // 2.5
    b = fixedpt_fromint(2); // 2.0
    result = fixedpt_mul(a, b);
    expected = fixedpt_fromint(5); // 5.0
    test_result("2.5 * 2.0 = 5.0", fixedpt_equal(result, expected));
    
    // 测试：0.5 * 0.5 = 0.25
    a = FIXEDPT_ONE_HALF; // 0.5
    b = FIXEDPT_ONE_HALF; // 0.5
    result = fixedpt_mul(a, b);
    expected = FIXEDPT_ONE >> 2; // 0.25 = 256 / 4 = 64
    test_result("0.5 * 0.5 = 0.25", fixedpt_equal(result, expected));
    
    // 测试：负数：-2.0 * 1.5 = -3.0
    a = -fixedpt_fromint(2); // -2.0
    b = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    result = fixedpt_mul(a, b);
    expected = -fixedpt_fromint(3); // -3.0
    test_result("-2.0 * 1.5 = -3.0", fixedpt_equal(result, expected));
    
    // 测试：乘以 0
    a = fixedpt_fromint(5);
    b = 0;
    result = fixedpt_mul(a, b);
    expected = 0;
    test_result("5 * 0 = 0", fixedpt_equal(result, expected));
    
    // 测试：乘以 1
    a = fixedpt_fromint(7);
    b = FIXEDPT_ONE; // 1.0
    result = fixedpt_mul(a, b);
    expected = fixedpt_fromint(7);
    test_result("7 * 1.0 = 7", fixedpt_equal(result, expected));
    
    // === 新增测试用例：更多小数结果的乘法 ===
    
    // 测试：1.5 * 1.5 = 2.25
    a = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    b = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    result = fixedpt_mul(a, b);
    expected = fixedpt_fromint(2) + (FIXEDPT_ONE >> 2); // 2.25
    test_result("1.5 * 1.5 = 2.25", fixedpt_equal(result, expected));
    
    // 测试：2.5 * 1.5 = 3.75
    a = fixedpt_fromint(2) + FIXEDPT_ONE_HALF; // 2.5
    b = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    result = fixedpt_mul(a, b);
    expected = fixedpt_fromint(3) + (FIXEDPT_ONE >> 1) + (FIXEDPT_ONE >> 2); // 3.75
    test_result("2.5 * 1.5 = 3.75", fixedpt_equal(result, expected));
    
    // 测试：0.25 * 0.5 = 0.125
    a = FIXEDPT_ONE >> 2; // 0.25
    b = FIXEDPT_ONE_HALF; // 0.5
    result = fixedpt_mul(a, b);
    expected = FIXEDPT_ONE >> 3; // 0.125 = 32
    test_result("0.25 * 0.5 = 0.125", fixedpt_equal(result, expected));
    
    // 测试：0.75 * 0.5 = 0.375
    a = (FIXEDPT_ONE >> 1) + (FIXEDPT_ONE >> 2); // 0.75
    b = FIXEDPT_ONE_HALF; // 0.5
    result = fixedpt_mul(a, b);
    expected = (FIXEDPT_ONE >> 2) + (FIXEDPT_ONE >> 3); // 0.375 = 96
    test_result("0.75 * 0.5 = 0.375", fixedpt_equal(result, expected));
    
    // 测试：负数：-1.5 * 2.0 = -3.0
    a = -(fixedpt_fromint(1) + FIXEDPT_ONE_HALF); // -1.5
    b = fixedpt_fromint(2); // 2.0
    result = fixedpt_mul(a, b);
    expected = -fixedpt_fromint(3); // -3.0
    test_result("-1.5 * 2.0 = -3.0", fixedpt_equal(result, expected));
    
    // 测试：负数小数结果：-1.5 * 1.5 = -2.25
    a = -(fixedpt_fromint(1) + FIXEDPT_ONE_HALF); // -1.5
    b = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    result = fixedpt_mul(a, b);
    expected = -(fixedpt_fromint(2) + (FIXEDPT_ONE >> 2)); // -2.25
    test_result("-1.5 * 1.5 = -2.25", fixedpt_equal(result, expected));
}

// 测试 fixedpt_div：两个定点数相除
void test_fixedpt_div() {
    printf("\n=== Testing fixedpt_div ===\n");
    
    // 测试：6.0 / 2.0 = 3.0
    fixedpt a = fixedpt_fromint(6); // 6.0
    fixedpt b = fixedpt_fromint(2); // 2.0
    fixedpt result = fixedpt_div(a, b);
    fixedpt expected = fixedpt_fromint(3); // 3.0
    test_result("6.0 / 2.0 = 3.0", fixedpt_equal(result, expected));
    
    // 测试：5.0 / 2.0 = 2.5
    a = fixedpt_fromint(5); // 5.0
    b = fixedpt_fromint(2); // 2.0
    result = fixedpt_div(a, b);
    expected = fixedpt_fromint(2) + FIXEDPT_ONE_HALF; // 2.5
    test_result("5.0 / 2.0 = 2.5", fixedpt_equal(result, expected));
    
    // 测试：1.5 / 0.5 = 3.0
    a = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    b = FIXEDPT_ONE_HALF; // 0.5
    result = fixedpt_div(a, b);
    expected = fixedpt_fromint(3); // 3.0
    test_result("1.5 / 0.5 = 3.0", fixedpt_equal(result, expected));
    
    // 测试：负数：-6.0 / 2.0 = -3.0
    a = -fixedpt_fromint(6); // -6.0
    b = fixedpt_fromint(2); // 2.0
    result = fixedpt_div(a, b);
    expected = -fixedpt_fromint(3); // -3.0
    test_result("-6.0 / 2.0 = -3.0", fixedpt_equal(result, expected));
    
    // 测试：除以 1.0
    a = fixedpt_fromint(5);
    b = FIXEDPT_ONE; // 1.0
    result = fixedpt_div(a, b);
    expected = fixedpt_fromint(5);
    test_result("5 / 1.0 = 5", fixedpt_equal(result, expected));
    
    // === 新增测试用例：更多小数结果的除法 ===
    
    // 测试：3.0 / 2.0 = 1.5
    a = fixedpt_fromint(3); // 3.0
    b = fixedpt_fromint(2); // 2.0
    result = fixedpt_div(a, b);
    expected = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    test_result("3.0 / 2.0 = 1.5", fixedpt_equal(result, expected));
    
    // 测试：7.0 / 4.0 = 1.75
    a = fixedpt_fromint(7); // 7.0
    b = fixedpt_fromint(4); // 4.0
    result = fixedpt_div(a, b);
    expected = fixedpt_fromint(1) + (FIXEDPT_ONE >> 1) + (FIXEDPT_ONE >> 2); // 1.75
    test_result("7.0 / 4.0 = 1.75", fixedpt_equal(result, expected));
    
    // 测试：9.0 / 4.0 = 2.25
    a = fixedpt_fromint(9); // 9.0
    b = fixedpt_fromint(4); // 4.0
    result = fixedpt_div(a, b);
    expected = fixedpt_fromint(2) + (FIXEDPT_ONE >> 2); // 2.25
    test_result("9.0 / 4.0 = 2.25", fixedpt_equal(result, expected));
    
    // 测试：1.0 / 3.0 ≈ 0.333...
    a = fixedpt_fromint(1); // 1.0
    b = fixedpt_fromint(3); // 3.0
    result = fixedpt_div(a, b);
    expected = FIXEDPT_ONE / 3; // 0.333... ≈ 85 (256/3 ≈ 85.33)
    test_result("1.0 / 3.0 ≈ 0.333", fixedpt_equal(result, expected));
    
    // 测试：1.0 / 4.0 = 0.25
    a = fixedpt_fromint(1); // 1.0
    b = fixedpt_fromint(4); // 4.0
    result = fixedpt_div(a, b);
    expected = FIXEDPT_ONE >> 2; // 0.25 = 64
    test_result("1.0 / 4.0 = 0.25", fixedpt_equal(result, expected));
    
    // 测试：3.0 / 4.0 = 0.75
    a = fixedpt_fromint(3); // 3.0
    b = fixedpt_fromint(4); // 4.0
    result = fixedpt_div(a, b);
    expected = (FIXEDPT_ONE >> 1) + (FIXEDPT_ONE >> 2); // 0.75 = 192
    test_result("3.0 / 4.0 = 0.75", fixedpt_equal(result, expected));
    
    // 测试：负数小数结果：-5.0 / 2.0 = -2.5
    a = -fixedpt_fromint(5); // -5.0
    b = fixedpt_fromint(2); // 2.0
    result = fixedpt_div(a, b);
    expected = -(fixedpt_fromint(2) + FIXEDPT_ONE_HALF); // -2.5
    test_result("-5.0 / 2.0 = -2.5", fixedpt_equal(result, expected));
    
    // 测试：负数小数结果：-3.0 / 4.0 = -0.75
    a = -fixedpt_fromint(3); // -3.0
    b = fixedpt_fromint(4); // 4.0
    result = fixedpt_div(a, b);
    expected = -((FIXEDPT_ONE >> 1) + (FIXEDPT_ONE >> 2)); // -0.75
    test_result("-3.0 / 4.0 = -0.75", fixedpt_equal(result, expected));
    
    // 测试：小数除以小数：2.5 / 0.5 = 5.0
    a = fixedpt_fromint(2) + FIXEDPT_ONE_HALF; // 2.5
    b = FIXEDPT_ONE_HALF; // 0.5
    result = fixedpt_div(a, b);
    expected = fixedpt_fromint(5); // 5.0
    test_result("2.5 / 0.5 = 5.0", fixedpt_equal(result, expected));
    
    // 测试：小数除以小数：1.25 / 0.25 = 5.0
    a = fixedpt_fromint(1) + (FIXEDPT_ONE >> 2); // 1.25
    b = FIXEDPT_ONE >> 2; // 0.25
    result = fixedpt_div(a, b);
    expected = fixedpt_fromint(5); // 5.0
    test_result("1.25 / 0.25 = 5.0", fixedpt_equal(result, expected));
    
    // 测试：小数除以小数：0.75 / 0.25 = 3.0
    a = (FIXEDPT_ONE >> 1) + (FIXEDPT_ONE >> 2); // 0.75
    b = FIXEDPT_ONE >> 2; // 0.25
    result = fixedpt_div(a, b);
    expected = fixedpt_fromint(3); // 3.0
    test_result("0.75 / 0.25 = 3.0", fixedpt_equal(result, expected));
}

// 测试 fixedpt_abs：绝对值
void test_fixedpt_abs() {
    printf("\n=== Testing fixedpt_abs ===\n");
    
    // 测试：正数的绝对值
    fixedpt a = fixedpt_fromint(5);
    fixedpt result = fixedpt_abs(a);
    fixedpt expected = fixedpt_fromint(5);
    test_result("abs(5) = 5", fixedpt_equal(result, expected));
    
    // 测试：负数的绝对值
    a = -fixedpt_fromint(5);
    result = fixedpt_abs(a);
    expected = fixedpt_fromint(5);
    test_result("abs(-5) = 5", fixedpt_equal(result, expected));
    
    // 测试：0 的绝对值
    a = 0;
    result = fixedpt_abs(a);
    expected = 0;
    test_result("abs(0) = 0", fixedpt_equal(result, expected));
    
    // 测试：正小数的绝对值
    a = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    result = fixedpt_abs(a);
    expected = fixedpt_fromint(1) + FIXEDPT_ONE_HALF;
    test_result("abs(1.5) = 1.5", fixedpt_equal(result, expected));
    
    // 测试：负小数的绝对值
    a = -(fixedpt_fromint(1) + FIXEDPT_ONE_HALF); // -1.5
    result = fixedpt_abs(a);
    expected = fixedpt_fromint(1) + FIXEDPT_ONE_HALF;
    test_result("abs(-1.5) = 1.5", fixedpt_equal(result, expected));
}

// 测试 fixedpt_floor：向下取整
void test_fixedpt_floor() {
    printf("\n=== Testing fixedpt_floor ===\n");
    
    // 测试：整数向下取整
    fixedpt a = fixedpt_fromint(5);
    fixedpt result = fixedpt_floor(a);
    fixedpt expected = fixedpt_fromint(5);
    test_result("floor(5) = 5", fixedpt_equal(result, expected));
    
    // 测试：正小数向下取整：1.5 -> 1.0
    a = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    result = fixedpt_floor(a);
    expected = fixedpt_fromint(1); // 1.0
    test_result("floor(1.5) = 1.0", fixedpt_equal(result, expected));
    
    // 测试：正小数向下取整：2.75 -> 2.0
    a = fixedpt_fromint(2) + (FIXEDPT_ONE >> 1) + (FIXEDPT_ONE >> 2); // 2.75
    result = fixedpt_floor(a);
    expected = fixedpt_fromint(2); // 2.0
    test_result("floor(2.75) = 2.0", fixedpt_equal(result, expected));
    
    // 测试：负数向下取整：-1.5 -> -2.0
    a = -(fixedpt_fromint(1) + FIXEDPT_ONE_HALF); // -1.5
    result = fixedpt_floor(a);
    expected = -fixedpt_fromint(2); // -2.0
    test_result("floor(-1.5) = -2.0", fixedpt_equal(result, expected));
    
    // 测试：负数向下取整：-2.25 -> -3.0
    a = -(fixedpt_fromint(2) + (FIXEDPT_ONE >> 2)); // -2.25
    result = fixedpt_floor(a);
    expected = -fixedpt_fromint(3); // -3.0
    test_result("floor(-2.25) = -3.0", fixedpt_equal(result, expected));
    
    // 测试：0
    a = 0;
    result = fixedpt_floor(a);
    expected = 0;
    test_result("floor(0) = 0", fixedpt_equal(result, expected));
}

// 测试 fixedpt_ceil：向上取整
void test_fixedpt_ceil() {
    printf("\n=== Testing fixedpt_ceil ===\n");
    
    // 测试：整数向上取整
    fixedpt a = fixedpt_fromint(5);
    fixedpt result = fixedpt_ceil(a);
    fixedpt expected = fixedpt_fromint(5);
    test_result("ceil(5) = 5", fixedpt_equal(result, expected));
    
    // 测试：正小数向上取整：1.5 -> 2.0
    a = fixedpt_fromint(1) + FIXEDPT_ONE_HALF; // 1.5
    result = fixedpt_ceil(a);
    expected = fixedpt_fromint(2); // 2.0
    test_result("ceil(1.5) = 2.0", fixedpt_equal(result, expected));
    
    // 测试：正小数向上取整：2.25 -> 3.0
    a = fixedpt_fromint(2) + (FIXEDPT_ONE >> 2); // 2.25
    result = fixedpt_ceil(a);
    expected = fixedpt_fromint(3); // 3.0
    test_result("ceil(2.25) = 3.0", fixedpt_equal(result, expected));
    
    // 测试：负数向上取整：-1.5 -> -1.0
    a = -(fixedpt_fromint(1) + FIXEDPT_ONE_HALF); // -1.5
    result = fixedpt_ceil(a);
    expected = -fixedpt_fromint(1); // -1.0
    test_result("ceil(-1.5) = -1.0", fixedpt_equal(result, expected));
    
    // 测试：负数向上取整：-2.75 -> -2.0
    a = -(fixedpt_fromint(2) + (FIXEDPT_ONE >> 1) + (FIXEDPT_ONE >> 2)); // -2.75
    result = fixedpt_ceil(a);
    expected = -fixedpt_fromint(2); // -2.0
    test_result("ceil(-2.75) = -2.0", fixedpt_equal(result, expected));
    
    // 测试：0
    a = 0;
    result = fixedpt_ceil(a);
    expected = 0;
    test_result("ceil(0) = 0", fixedpt_equal(result, expected));
}

int main() {
    printf("========================================\n");
    printf("Fixed Point Arithmetic Test Suite\n");
    printf("========================================\n");
    
    test_fixedpt_muli();
    test_fixedpt_divi();
    test_fixedpt_mul();
    test_fixedpt_div();
    test_fixedpt_abs();
    test_fixedpt_floor();
    test_fixedpt_ceil();
    
    printf("\n========================================\n");
    printf("All tests completed!\n");
    printf("========================================\n");
    
    return 0;
}

