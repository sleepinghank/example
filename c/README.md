# C Demo

## 一、原相组合键

> 原相原生组合键开发改动内容较多，解耦不够。简单学习了QMK的Combos方式，重新封装。

### 1. 文件说明

只有两个文件。

| 文件名          | 说明   |
| --------------- | ------ |
| process_combo.c |        |
| process_combo.h | 头文件 |

### 2. 组件调用

组件遵循解耦原则，最小入侵。但还是会修改按键判断部分。

| 函数名称                                                     | 说明         |
| ------------------------------------------------------------ | ------------ |
| void combo_task(void)                                        | 组件调用   |

#### 2.1 combo_task 调用

需要在keyboard.c中get_kb_rpt()方法中进行调用

```c
    debounce_proc(k_st);
	combo_task(k_st);
    report_update_proc(k_st);
```

#### 2.2 其他更改

linkedlist.h文件更改结构体，添加新方法

```c
typedef struct
{
    uint16_t key_code;
    uint8_t cycle;
    uint8_t is_report;
    report_t report_type;
} bouncing_data_t;
void deactivate(uint16_t data, list_t* list);
void del_all_child(list_t* list);
```

linkedlist.c文件同步更改

```c
void del_all_child(list_t* list)
{
    if (list == NULL || list->head == NULL)
        return;
    {
        node_t* current = list->head;
        node_t* next = current;

        while (current != NULL)
        {
            next = current->next;
            free(current);
            current = next;
        }
    }

}

void deactivate(uint16_t data, list_t* list)
{
    node_t* current = list->head;

    while (current != NULL)
    {
        if (current->data.key_code == data)
        {
            current->data.is_report = 0;
            return;
        }

        current = current->next;
    }
}

void add(uint16_t data, list_t* list)
    bouncing_data.key_code = data;
    bouncing_data.is_report = 1; // 添加
```

keyboard.c中同步增加_key_code_list_extend

```c
list_t* _key_code_list;          // current press key
list_t* _key_code_list_extend;          // extend press key

 destroy(_key_code_list_extend);
_key_code_list_extend = make_list_proc();

void report_update_proc(key_update_st_t _keyUpdateSt){
    uint8_t is_extend = 0;
    if (current == null){
        current = _key_code_list_extend->head;
        is_extend = 1;
    }
    if (_keyUpdateSt != GHOST_KEY)
    {
        while (current != NULL)
        {
            curr_tmp = current;
            current = current->next;
            if (current == NULL && is_extend == 0) {
                current = _key_code_list_extend->head;
                is_extend = 1;
            }
            if (curr_tmp->data.is_report == 0) {
                continue;
            }
            uint16_t keycode = curr_tmp->data.key_code;
        }
    }
}
```



#### 2.3 使用Demo

```c
enum combos {
    AB_ESC,
    CD_ESC,
    EF_ESC,
    combos_end,
};
uint8_t number_of_combos = combos_end;

// 组合键触发后的回调函数，arr为触发的按键
uint8_t BtnCallback1(uint8_t* add_keys){
    uint8_t idx = 0;
    add_keys[idx++] = KB_1;
    add_keys[idx++] = KB_2;
    return idx;
}
// 组合键触发后的回调函数，arr为触发的按键
uint8_t BtnCallback2(uint8_t* add_keys){
    uint8_t idx = 0;
    add_keys[idx++] = KB_3;
    add_keys[idx++] = KB_4;
    return idx;
}


const uint16_t PROGMEM ab_combo[] = {KB_A, KB_B, COMBO_END};
const uint16_t PROGMEM cd_combo[] = {KB_C, KB_D, COMBO_END};
const uint16_t PROGMEM ef_combo[] = {KB_E, KB_F, COMBO_END};

combo_t key_combos[] = {
        [AB_ESC] = COMBO(ab_combo,PRESS_DOWN,BtnCallback1),
        [CD_ESC] = COMBO2(cd_combo,PRESS_UP,BtnCallback1,LONG_PRESS_HOLD,BtnCallback2),
        [EF_ESC] = COMBO_LONG_TICKS(ef_combo,600,LONG_PRESS_HOLD, BtnCallback1),
};
```