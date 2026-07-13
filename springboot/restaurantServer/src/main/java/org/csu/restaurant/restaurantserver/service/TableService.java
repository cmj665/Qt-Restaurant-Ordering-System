package org.csu.restaurant.restaurantserver.service;

import org.csu.restaurant.restaurantserver.entity.DiningTable;

import java.util.List;

public interface TableService {

    List<DiningTable> findAll();

    int updateStatus(Integer id,Integer status);

    boolean clean(Integer id);
}
