package org.csu.restaurant.restaurantserver.service;

import org.csu.restaurant.restaurantserver.entity.Dish;

import java.util.List;

public interface DishService {

    List<Dish> findAll();

    int addDish(Dish dish);

    int updateDish(Dish dish);

    int deleteDish(Integer id);

}
