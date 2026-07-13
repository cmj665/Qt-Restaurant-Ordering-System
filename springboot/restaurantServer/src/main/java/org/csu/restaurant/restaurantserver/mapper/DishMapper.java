package org.csu.restaurant.restaurantserver.mapper;

import org.csu.restaurant.restaurantserver.entity.Dish;
import org.apache.ibatis.annotations.Mapper;

import java.util.List;

@Mapper
public interface DishMapper {

    //查询全部菜品
    List<Dish> findAll();

    //新增菜品
    int addDish(Dish dish);

    //修改菜品
    int updateDish(Dish dish);

    //逻辑删除
    int deleteDish(Integer id);

    //根据id查询菜品
    Dish findById(Integer id);

    //扣库存
    int reduceStock(Integer id,Integer count);


}
