package org.csu.restaurant.restaurantserver.service.impl;

import org.csu.restaurant.restaurantserver.entity.Dish;
import org.csu.restaurant.restaurantserver.entity.DishCategory;
import org.csu.restaurant.restaurantserver.mapper.DishMapper;
import org.csu.restaurant.restaurantserver.service.DishService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;
import java.math.BigDecimal;

@Service
public class DishServiceImpl implements DishService{

    @Autowired
    private DishMapper dishMapper;

    @Override
    public List<Dish> findAll(){
        return dishMapper.findAll();
    }

    @Override
    public List<Dish> findAllForAdmin(){
        return dishMapper.findAllForAdmin();
    }

    @Override
    public List<DishCategory> findCategories(){
        return dishMapper.findCategories();
    }

    @Override
    public int addDish(Dish dish) {
        validateDish(dish, false);
        int affected = dishMapper.addDish(dish);
        if(affected == 0) throw new IllegalStateException("新增菜品失败");
        return affected;
    }

    @Override
    public int updateDish(Dish dish) {
        validateDish(dish, true);
        requireActive(dish.getId());
        int affected = dishMapper.updateDish(dish);
        if(affected == 0) throw new IllegalStateException("菜品已发生变化，请刷新后重试");
        return affected;
    }

    @Override
    public int deleteDish(Integer id) {
        requireActive(id);
        int affected = dishMapper.deleteDish(id);
        if(affected == 0) throw new IllegalStateException("菜品已下架，请刷新列表");
        return affected;
    }

    @Override
    public void restoreDish(Integer id) {
        if(id == null) throw new IllegalArgumentException("菜品ID不能为空");
        Dish dish=dishMapper.findById(id);
        if(dish == null) throw new IllegalArgumentException("菜品不存在");
        if(!Integer.valueOf(1).equals(dish.getIsDeleted())) throw new IllegalStateException("菜品已经处于上架状态");
        if(dishMapper.restoreDish(id) == 0) throw new IllegalStateException("上架失败，请刷新后重试");
    }

    @Override
    public List<Dish> findLowStock(){

        return dishMapper.findLowStock();

    }

    @Override
    public void adjustStock(Integer dishId, Integer stock) {
        requireActive(dishId);
        if(stock == null || stock < 0) throw new IllegalArgumentException("库存不能小于0");
        if(dishMapper.updateStock(dishId, stock) == 0) throw new IllegalStateException("库存调整失败，请刷新后重试");
    }

    private void validateDish(Dish dish, boolean requireId) {
        if(dish == null || (requireId && dish.getId() == null)) throw new IllegalArgumentException("菜品参数无效");
        if(dish.getCatId() == null || dish.getName() == null || dish.getName().trim().isEmpty()) throw new IllegalArgumentException("分类和菜品名称不能为空");
        dish.setName(dish.getName().trim());
        if(dish.getPrice() == null || dish.getPrice().compareTo(BigDecimal.ZERO) <= 0) throw new IllegalArgumentException("菜品价格必须大于0");
        if(dish.getStock() == null || dish.getStock() < 0) throw new IllegalArgumentException("库存不能小于0");
        if(dish.getPicture() == null) dish.setPicture("");
        if(dish.getDescription() == null) dish.setDescription("");
    }

    private Dish requireActive(Integer id) {
        if(id == null) throw new IllegalArgumentException("菜品ID不能为空");
        Dish dish=dishMapper.findById(id);
        if(dish == null || Integer.valueOf(1).equals(dish.getIsDeleted())) throw new IllegalArgumentException("菜品不存在或已下架");
        return dish;
    }


}
