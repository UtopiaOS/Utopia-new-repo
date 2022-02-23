


#include <covenant/std.h>

ctype_status
c_dyn_idxcat(ctype_arr *array, usize pos, void *v, usize obj_num, usize obj_size)
{
    usize len;
    uchar *target;

    if (c_dyn_ready(array, obj_num, obj_size) < 0)
        return -1;
    
    len = c_arr_bytes(array);
    if (!(target = c_dyn_alloc(array, pos, obj_size)))
        return -1;
    array->length = len;
    obj_num *= obj_size;
    if (!pos || (pos = (pos - 1) * obj_size) < len)
        c_mem_cpy(target + obj_num, array->length, target);
    c_mem_cpy(target, obj_num, v);
    array->length += obj_num;
    array->members[array->length] = 0;
    return 0;
}