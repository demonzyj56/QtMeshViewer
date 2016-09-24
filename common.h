#ifndef COMMON_H
#define COMMON_H

template<typename Object>
void SafeDelete(Object *obj) {
    if (obj) {
        delete obj;
        obj = nullptr;
    }
}

#endif // COMMON_H
