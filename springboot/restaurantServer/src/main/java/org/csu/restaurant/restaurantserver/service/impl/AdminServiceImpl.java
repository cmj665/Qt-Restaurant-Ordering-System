package org.csu.restaurant.restaurantserver.service.impl;

import org.csu.restaurant.restaurantserver.entity.Admin;
import org.csu.restaurant.restaurantserver.mapper.AdminMapper;
import org.csu.restaurant.restaurantserver.service.AdminService;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HexFormat;
import java.util.Locale;

@Service
@RequiredArgsConstructor
public class AdminServiceImpl implements AdminService {
    private final AdminMapper adminMapper;

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

    //查询密保问题
    @Override
    public String getSecurityQuestion(String username) {
        Admin admin = requireAdmin(username);
        if (admin.getSecurityQuestion() == null || admin.getSecurityQuestion().isBlank()
                || admin.getSecurityAnswerHash() == null || admin.getSecurityAnswerHash().isBlank()) {
            throw new IllegalStateException("该管理员尚未设置密保问题，请联系数据库管理员");
        }
        return admin.getSecurityQuestion();
    }

    //验证答案并重置密码
    @Override
    public void resetPassword(String username, String securityAnswer, String newPassword) {
        Admin admin = requireAdmin(username);
        if (securityAnswer == null || securityAnswer.isBlank()) {
            throw new IllegalArgumentException("请输入密保答案");
        }
        if (newPassword == null || newPassword.length() < 6) {
            throw new IllegalArgumentException("新密码至少需要6位");
        }
        if (admin.getSecurityAnswerHash() == null || !admin.getSecurityAnswerHash().equalsIgnoreCase(hashAnswer(securityAnswer))) {
            throw new IllegalArgumentException("密保答案错误");
        }
        if (adminMapper.updatePassword(admin.getUsername(), newPassword) != 1) {
            throw new IllegalStateException("密码重置失败，请稍后重试");
        }
    }

    private Admin requireAdmin(String username) {
        if (username == null || username.isBlank()) throw new IllegalArgumentException("请输入管理员账号");
        Admin admin = adminMapper.findByUsername(username.trim());
        if (admin == null) throw new IllegalArgumentException("管理员账号不存在");
        return admin;
    }

    //密保答案加密
    private String hashAnswer(String answer) {
        try {
            String normalized = answer.trim().toLowerCase(Locale.ROOT);
            //使用 SHA-256 计算哈希值
            return HexFormat.of().formatHex(MessageDigest.getInstance("SHA-256")
                    .digest(normalized.getBytes(StandardCharsets.UTF_8)));
        } catch (NoSuchAlgorithmException exception) {
            throw new IllegalStateException("服务器无法校验密保答案", exception);
        }
    }






}
