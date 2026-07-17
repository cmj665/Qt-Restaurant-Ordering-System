package org.csu.restaurant.restaurantserver.controller;

import org.csu.restaurant.restaurantserver.dto.PaymentDTO;
import org.csu.restaurant.restaurantserver.service.PaymentService;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/payment")
@RequiredArgsConstructor
public class PaymentController {
    private final PaymentService paymentService;

    @PostMapping("/pay")
    public String pay(@RequestBody PaymentDTO paymentDTO){
        boolean result = paymentService.pay(paymentDTO);

        return result?"支付成功":"支付失败";
    }


}
