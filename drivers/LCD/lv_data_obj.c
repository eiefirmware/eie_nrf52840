/**
 * @file lv_data_obj.c
 *
 */

/***********************************************************************
 * Includes
 **********************************************************************/

#include <lvgl.h>
#include <stddef.h>
#include <string.h>
#include <zephyr/kernel.h>

#include "core/lv_obj_class_private.h"
#include "core/lv_obj_private.h"

/***********************************************************************
 * Types
 **********************************************************************/

typedef struct _lv_data_obj_t {
  lv_obj_t obj;
  void *data;
} lv_data_obj_t;

/***********************************************************************
 * Prototypes
 **********************************************************************/

static void lv_data_obj_constructor(const lv_obj_class_t *class_p,
                                    lv_obj_t *obj);
static void lv_data_obj_destructor(const lv_obj_class_t *class_p,
                                   lv_obj_t *obj);

/***********************************************************************
 * Variables
 **********************************************************************/

const lv_obj_class_t lv_data_obj_class = {
    .constructor_cb = lv_data_obj_constructor,
    .destructor_cb = lv_data_obj_destructor,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_data_obj_t),
    .base_class = &lv_obj_class,
    .name = "lv_data_obj",
};

/***********************************************************************
 * Functions
 **********************************************************************/

lv_obj_t *lv_data_obj_create(lv_obj_t *parent) {
  lv_obj_t *obj = lv_obj_class_create_obj(&lv_data_obj_class, parent);
  lv_obj_class_init_obj(obj);

  return obj;
}

bool lv_data_obj_allocate(lv_obj_t const *obj, size_t size) {
  if (obj == NULL) {
    return false;
  }
  lv_data_obj_t *data_obj = (lv_data_obj_t *)obj;
  data_obj->data = lv_malloc_zeroed(size);

  return data_obj->data != NULL;
}

lv_obj_t *lv_data_obj_create_alloc_assign(lv_obj_t *parent, void const *data,
                                          size_t size) {
  if (data == NULL) {
    return NULL;
  }
  lv_obj_t *obj = lv_data_obj_create(parent);
  if (lv_data_obj_allocate(obj, size)) {
    lv_data_obj_t *data_obj = (lv_data_obj_t *)obj;
    memcpy(data_obj->data, data, size);
  } else {
    lv_obj_delete(obj);
    obj = NULL;
  }

  return obj;
}

void *lv_data_obj_get_data_ptr(lv_obj_t const *obj) {
  lv_data_obj_t *data_obj = (lv_data_obj_t *)obj;
  return data_obj->data;
}

static void lv_data_obj_constructor(
    const lv_obj_class_t __attribute__((unused)) * class_p, lv_obj_t *obj) {
  lv_data_obj_t *data_obj = (lv_data_obj_t *)obj;
  data_obj->data = NULL;
}

static void lv_data_obj_destructor(
    const lv_obj_class_t __attribute__((unused)) * class_p, lv_obj_t *obj) {
  lv_data_obj_t *data_obj = (lv_data_obj_t *)obj;
  lv_free(data_obj->data);
}
