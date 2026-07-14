package org.csu.restaurant.restaurantserver.service;

import org.csu.restaurant.restaurantserver.entity.DiningTable;

import java.util.List;

public interface TableService {

    List<DiningTable> findAll();

    void changeStatus(Integer id,Integer status);

    void clean(Integer id);
}
