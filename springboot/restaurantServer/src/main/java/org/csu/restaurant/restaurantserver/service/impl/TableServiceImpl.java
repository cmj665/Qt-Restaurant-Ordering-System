package org.csu.restaurant.restaurantserver.service.impl;

import org.csu.restaurant.restaurantserver.entity.DiningTable;
import org.csu.restaurant.restaurantserver.mapper.TableMapper;
import org.csu.restaurant.restaurantserver.service.TableService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TableServiceImpl implements TableService{

    @Autowired
    private TableMapper tableMapper;


    @Override
    public List<DiningTable> findAll(){
        return tableMapper.findAll();
    }

    @Override
    public int updateStatus(Integer id,Integer status){
        return tableMapper.updateStatus(id,status);
    }

    @Override
    public boolean clean(Integer id) {
        int result = tableMapper.updateStatus(id,0);
        return result>0;
    }

}
