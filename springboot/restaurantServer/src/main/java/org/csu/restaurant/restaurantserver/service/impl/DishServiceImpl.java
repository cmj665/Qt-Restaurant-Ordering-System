package org.csu.restaurant.restaurantserver.service.impl;

import org.csu.restaurant.restaurantserver.entity.Dish;
import org.csu.restaurant.restaurantserver.mapper.DishMapper;
import org.csu.restaurant.restaurantserver.service.DishService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class DishServiceImpl implements DishService{

    @Autowired
    private DishMapper dishMapper;

    @Override
    public List<Dish> findAll(){
        return dishMapper.findAll();
    }

    @Override
    public int addDish(Dish dish) {
        return dishMapper.addDish(dish);
    }

    @Override
    public int updateDish(Dish dish) {
        return dishMapper.updateDish(dish);
    }

    @Override
    public int deleteDish(Integer id) {
        return dishMapper.deleteDish(id);
    }




}
