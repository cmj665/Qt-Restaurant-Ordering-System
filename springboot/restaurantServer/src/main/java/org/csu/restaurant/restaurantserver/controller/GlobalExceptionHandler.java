package org.csu.restaurant.restaurantserver.controller;


import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;

import java.util.HashMap;
import java.util.Map;

@RestControllerAdvice
public class GlobalExceptionHandler {

    @ExceptionHandler(RuntimeException.class)
    public Map<String,Object> handle(RuntimeException e){

        Map<String,Object> result=new HashMap<>();

        result.put("success",false);
        result.put("message",e.getMessage());

        return result;
    }




}
