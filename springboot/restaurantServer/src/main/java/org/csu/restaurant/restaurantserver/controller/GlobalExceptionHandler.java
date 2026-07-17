package org.csu.restaurant.restaurantserver.controller;


import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;

import java.util.HashMap;
import java.util.Map;

@RestControllerAdvice
/** 将业务异常转换为客户端可稳定解析的 JSON 错误结构。 */
public class GlobalExceptionHandler {

    @ExceptionHandler(IllegalStateException.class)
    /** 状态冲突（例如重复支付或重复下架）使用 HTTP 409。 */
    public ResponseEntity<Map<String,Object>> handleConflict(IllegalStateException e){
        Map<String,Object> result = new HashMap<>();
        result.put("success", false);
        result.put("message", e.getMessage());
        return ResponseEntity.status(HttpStatus.CONFLICT).body(result);
    }

    @ExceptionHandler(RuntimeException.class)
    /** 参数及一般业务错误统一作为 HTTP 400 返回，避免向客户端暴露堆栈。 */
    public ResponseEntity<Map<String,Object>> handle(RuntimeException e){

        Map<String,Object> result=new HashMap<>();

        result.put("success",false);
        result.put("message",e.getMessage());

        return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(result);
    }




}
