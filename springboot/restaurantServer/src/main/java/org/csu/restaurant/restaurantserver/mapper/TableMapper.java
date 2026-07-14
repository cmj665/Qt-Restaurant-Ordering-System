package org.csu.restaurant.restaurantserver.mapper;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.csu.restaurant.restaurantserver.entity.DiningTable;

import java.util.List;

@Mapper
public interface TableMapper {

    //查询所有桌台
    List<DiningTable> findAll();

    int updateStatusIfCurrent(@Param("id") Integer id,
                              @Param("currentStatus") Integer currentStatus,
                              @Param("status") Integer status);

    int markDining(Integer id);

    //查找桌台ID
    DiningTable findById(Integer id);

}
