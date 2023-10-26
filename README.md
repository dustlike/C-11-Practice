# C++11 Practice
在C++11標準下撰寫的計算機程式

# 使用說明
+ 只能輸入整數進行整數運算。輸入的整數最大99999999，但運算的數值範圍是INT32
+ 支援四則運算(`+-*/`)、取模運算(`%`)、指數運算(`^`)
+ 支援一元負號運算子，而且可以連續使用
+ 運算子優先度符合一般常識，可用括號`()`變更運算順序

# 範例
```
> 1+1
2
```

```
> (  1  +  2   )  * 3
9
```

```
> 1+-1
0
```
```
> 3^5
243
```
```
> 4*(-2+8)/8
3
```
```
> 83-(25-18)%17*(68+60)/16
27
```
```
> 1----2
3
```
```
> 1++++2
UnCalc: Missing operand
```
```
> 99999999+1
100000000
```
```
> 100000000-99999999
UnCalc: operand too big
```
```
> 1 1 +
UnCalc: Missing operator
```
```
> 1 (+1)*3
UnCalc: Missing operator before '('
```
```
> 2*(3+)4
UnCalc: Missing operand
```
```
> 2*()
UnCalc: Missing operand before ')'
```
```
> 1$1
UnCalc: Unknown operator
```
```
> 10000%((10-7)*(8+3))
1
```
```
> 1000%((10-7)*(8+3))
10
```
```
> 1000%((10-7)*(8+3)
UnCalc: Missing ')'
```
```
> 1000%(10-7)*(8+3))
UnCalc: Missing '('
```
```
> 
UnCalc: empty expression
```
```
> ()
UnCalc: Missing operand before ')'
```
```
> ()+1
UnCalc: Missing operand before ')'
```
```
> (4)+5
9
```
```
> 17/(15+(17-12)*(-10+(3+4)))
divided by zero in #UnCalc
```
```
> 17%(15+(17-12)*(-10+(3+4)))
modulo by zero in #UnCalc
```
