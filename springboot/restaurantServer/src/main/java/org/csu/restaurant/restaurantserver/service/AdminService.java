package org.csu.restaurant.restaurantserver.service;

import org.csu.restaurant.restaurantserver.entity.Admin;

public interface AdminService {

    Admin login(String username, String password);

}
