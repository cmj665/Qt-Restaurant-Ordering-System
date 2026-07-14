package org.csu.restaurant.restaurantserver.controller;

import org.csu.restaurant.restaurantserver.entity.Dish;
import org.csu.restaurant.restaurantserver.entity.DishCategory;
import org.csu.restaurant.restaurantserver.service.DishService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/dish")
public class DishController {

    @Autowired
    private DishService dishService;

    // 查询全部菜品
    @GetMapping("/list")
    public List<Dish> list(){

        return dishService.findAll();
    }

    @GetMapping("/categories")
    public List<DishCategory> categories(){
        return dishService.findCategories();
    }

    // 新增菜品
    @PostMapping("/add")
    public String add(@RequestBody Dish dish) {

        int result = dishService.addDish(dish);

        return result > 0 ? "添加成功" : "添加失败";
    }

    // 修改菜品
    @PostMapping("/update")
    public String update(@RequestBody Dish dish) {

        int result = dishService.updateDish(dish);

        return result > 0 ? "修改成功" : "修改失败";
    }

    // 删除菜品（逻辑删除）
    @PostMapping("/delete/{id}")
    public String delete(@PathVariable Integer id) {

        int result = dishService.deleteDish(id);

        return result > 0 ? "删除成功" : "删除失败";
    }

}
