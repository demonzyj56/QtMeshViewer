#ifndef LEOYOLO_COMMON_H
#define LEOYOLO_COMMON_H
#include <time.h>
#include <stdio.h>
#include <stdexcept>

#define NotImplemented do{\
    throw std::runtime_error("Not implemented!"); \
    }while(0)

template <typename data_type>
static data_type MIN(data_type a, data_type b) { return a<b ? a : b; }
template <typename data_type>
static data_type MAX(data_type a, data_type b) { return a>b ? a : b; }

// timing utility
static clock_t tp = clock();

static clock_t *tic() {
    tp = clock();
    return &tp;
}

static double toc(const clock_t *t = nullptr) {
    if (t != nullptr) {
        return static_cast<double>(clock() - *t) / CLOCKS_PER_SEC;
    } else {
        double elapsed_sec = static_cast<double>(clock() - tp) / CLOCKS_PER_SEC;
        printf("Elapsed time: %.4fs\n", elapsed_sec);
        return elapsed_sec;
   }
}

// template for safely deleting pointers.
template<typename Object>
void SafeDelete(Object *obj) {
    if (obj) {
        delete obj;
        obj = nullptr;
    }
}

#endif // LEOYOLO_COMMON_H
