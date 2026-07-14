package org.csu.restaurant.restaurantserver.service;

import org.csu.restaurant.restaurantserver.entity.Admin;

public interface AdminService {

    Admin login(String username, String password);
    String getSecurityQuestion(String username);
    void resetPassword(String username, String securityAnswer, String newPassword);

}
