package org.csu.restaurant.restaurantserver.service.impl;

import org.csu.restaurant.restaurantserver.entity.Admin;
import org.csu.restaurant.restaurantserver.mapper.AdminMapper;
import org.csu.restaurant.restaurantserver.service.AdminService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
public class AdminServiceImpl implements AdminService {

    @Autowired
    private AdminMapper adminMapper;

    @Override
    public Admin login(String username, String password){

        //根据账号查询
        Admin admin = adminMapper.findByUsername(username);

        //账号不存在
        if(admin==null)
        {
            throw new RuntimeException("账号不存在");
        }

        //密码错误
        if(!admin.getPassword().equals(password))
        {
            throw new RuntimeException("密码错误");
        }

        return admin;

    }






}
