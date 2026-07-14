package org.csu.restaurant.restaurantserver.mapper;

import org.apache.ibatis.annotations.Mapper;
import org.csu.restaurant.restaurantserver.entity.Admin;

@Mapper
public interface AdminMapper {

    //根据账号查询管理员
    Admin findByUsername(String username);

}
