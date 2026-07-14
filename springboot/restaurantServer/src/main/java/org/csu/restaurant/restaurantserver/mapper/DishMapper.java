package org.csu.restaurant.restaurantserver.mapper;

import org.csu.restaurant.restaurantserver.entity.Dish;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;

import java.util.List;
import org.csu.restaurant.restaurantserver.entity.DishCategory;

@Mapper
public interface DishMapper {

    //查询全部菜品
    List<Dish> findAll();
    List<Dish> findAllForAdmin();

    List<DishCategory> findCategories();

    //新增菜品
    int addDish(Dish dish);

    //修改菜品
    int updateDish(Dish dish);

    //逻辑删除
    int deleteDish(Integer id);
    int restoreDish(Integer id);

    //根据id查询菜品
    Dish findById(Integer id);

    //扣库存
    int reduceStock(@Param("id") Integer id,@Param("count") Integer count);
    int updateStock(@Param("id") Integer id,@Param("stock") Integer stock);
    int addStock(@Param("id") Integer id,@Param("count") Integer count);

    //查询库存不足菜品
    List<Dish> findLowStock();

}
