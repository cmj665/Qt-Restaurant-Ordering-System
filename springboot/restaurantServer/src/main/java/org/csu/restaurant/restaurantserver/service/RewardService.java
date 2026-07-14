package org.csu.restaurant.restaurantserver.service;
import org.csu.restaurant.restaurantserver.dto.RewardChanceDTO;
import org.csu.restaurant.restaurantserver.dto.RewardResultDTO;
public interface RewardService {
    RewardChanceDTO chancesByTable(Integer tableId);
    RewardChanceDTO chancesByOrder(Integer orderId);
    RewardResultDTO draw(Integer orderId);
}
