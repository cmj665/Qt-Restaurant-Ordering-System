package org.csu.restaurant.restaurantserver.service;

import org.csu.restaurant.restaurantserver.dto.PaymentDTO;

public interface PaymentService {

    boolean pay(PaymentDTO paymentDTO);
}
