#ifndef LV_DATA_OBJ_H
#define LV_DATA_OBJ_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>

/**
 * @brief Create LV data object that is a child of parent
 *
 * @param[in] parent The parent object. If parent is null, a new screen
 *   will be created to attach the child to
 * @return lv_obj_t* The data object.
 */
lv_obj_t* lv_data_obj_create(lv_obj_t* parent);

/**
 * @brief Allocate memory space in a LV data object
 *
 * @param[in] obj The object data is being allocated to
 * @param[in] size The size of memory to be allocated
 * @return true if memory is successfully allocated
 * @return false if memory is not allocated
 */
bool lv_data_obj_allocate(lv_obj_t const* obj, size_t size);

/**
 * @brief Create a new LV data object, allocate memory for it and copy
 * data into said memory.
 *
 * @param[in] parent The parent object
 * @param[in] data Pointer to data being copied in
 * @param[in] size Size of memory to be allocated/Size of data
 * @return lv_obj_t* The data object. If creation fails, null will be returned
 */
lv_obj_t* lv_data_obj_create_alloc_assign(lv_obj_t* parent, void const* data,
                                          size_t size);

/**
 * @brief Get data pointer back from data object
 *
 * @param obj The object
 * @return void* Pointer to the data
 */
void* lv_data_obj_get_data_ptr(lv_obj_t const* obj);

#ifdef __cplusplus
}
#endif

#endif
