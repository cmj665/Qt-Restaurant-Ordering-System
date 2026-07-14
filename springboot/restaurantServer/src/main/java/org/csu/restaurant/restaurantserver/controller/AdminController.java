package org.csu.restaurant.restaurantserver.controller;

import org.csu.restaurant.restaurantserver.entity.Admin;
import org.csu.restaurant.restaurantserver.service.AdminService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.LinkedHashMap;
import java.util.Map;

@RestController
@RequestMapping("/admin")
public class AdminController {

    @Autowired
    private AdminService adminService;

    @PostMapping("/login")
    public Map<String, Object> login(@RequestBody Admin admin){
        Admin authenticated = adminService.login(admin.getUsername(), admin.getPassword());
        Map<String, Object> result = new LinkedHashMap<>();
        result.put("success", true);
        result.put("id", authenticated.getId());
        result.put("username", authenticated.getUsername());
        result.put("role", authenticated.getRole());
        return result;
    }

    @GetMapping("/security-question")
    public Map<String, Object> securityQuestion(@RequestParam String username) {
        Map<String, Object> result = new LinkedHashMap<>();
        result.put("success", true);
        result.put("username", username.trim());
        result.put("question", adminService.getSecurityQuestion(username));
        return result;
    }

    @PostMapping("/reset-password")
    public Map<String, Object> resetPassword(@RequestBody Map<String, String> request) {
        adminService.resetPassword(request.get("username"), request.get("securityAnswer"), request.get("newPassword"));
        Map<String, Object> result = new LinkedHashMap<>();
        result.put("success", true);
        result.put("message", "密码重置成功，请使用新密码登录");
        return result;
    }

}
