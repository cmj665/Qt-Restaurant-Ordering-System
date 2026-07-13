package org.csu.restaurant.restaurantserver.mapper;

import org.apache.ibatis.annotations.Mapper;
import org.csu.restaurant.restaurantserver.entity.DiningTable;

import java.util.List;

@Mapper
public interface TableMapper {

    //查询所有桌台
    List<DiningTable> findAll();

    //修改桌台状态
    int updateStatus(Integer id,Integer status);

    //查找桌台ID
    DiningTable findById(Integer id);

}
