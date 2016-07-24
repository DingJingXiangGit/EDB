#ifndef RM_OPERATOR_H
#define RM_OPERATOR_H

bool equal_int(void *left, void *right,
               __attribute__((unused)) int length) {
    return *(static_cast<int*>(left)) == *(static_cast<int*>(right));
}

bool equal_float(void *left, void *right,
               __attribute__((unused)) int length) {
    return *(static_cast<float*>(left)) == *(static_cast<float*>(right));
}

bool equal_str(void *left, void *right, int length) {
    return strncmp(static_cast<char*>(left),
                   static_cast<char*>(right),
                   length) == 0;
}

bool no_equal_int(void *left, void *right,
               __attribute__((unused)) int length) {
    return *(static_cast<int*>(left)) != *(static_cast<int*>(right));
}

bool no_equal_float(void *left, void *right,
               __attribute__((unused)) int length) {
    return *(static_cast<float*>(left)) != *(static_cast<float*>(right));
}

bool no_equal_str(void *left, void *right, int length) {
    return strncmp(static_cast<char*>(left),
                   static_cast<char*>(right),
                   length) != 0;
}

bool greater_than_int(void *left, void *right,
               __attribute__((unused)) int length) {
    return *(static_cast<int*>(left)) > *(static_cast<int*>(right));
}

bool greater_than_float(void *left, void *right,
               __attribute__((unused)) int length) {
    return *(static_cast<float*>(left)) > *(static_cast<float*>(right));
}

bool greater_than_str(void *left, void *right, int length) {
    return strncmp(static_cast<char*>(left),
                   static_cast<char*>(right),
                   length) > 0;
}

bool greater_or_equal_int(void *left, void *right,
               __attribute__((unused)) int length) {
    return *(static_cast<int*>(left)) >= *(static_cast<int*>(right));
}

bool greater_or_equal_float(void *left, void *right,
               __attribute__((unused)) int length) {
    return *(static_cast<float*>(left)) >= *(static_cast<float*>(right));
}

bool greater_or_equal_str(void *left, void *right, int length) {
    return strncmp(static_cast<char*>(left),
                   static_cast<char*>(right),
                   length) >= 0;
}

bool less_than_int(void *left, void *right,
               __attribute__((unused)) int length) {
    return *(static_cast<int*>(left)) < *(static_cast<int*>(right));
}

bool less_than_float(void *left, void *right,
               __attribute__((unused)) int length) {
    return *(static_cast<float*>(left)) < *(static_cast<float*>(right));
}

bool less_than_str(void *left, void *right, int length) {
    return strncmp(static_cast<char*>(left),
                   static_cast<char*>(right),
                   length) < 0;
}


bool less_or_equal_int(void *left, void *right,
               __attribute__((unused)) int length) {
    return *(static_cast<int*>(left)) <= *(static_cast<int*>(right));
}

bool less_or_equal_float(void *left, void *right,
               __attribute__((unused)) int length) {
    return *(static_cast<float*>(left)) <= *(static_cast<float*>(right));
}

bool less_or_equal_str(void *left, void *right, int length) {
    return strncmp(static_cast<char*>(left),
                   static_cast<char*>(right),
                   length) <= 0;
}

#endif
