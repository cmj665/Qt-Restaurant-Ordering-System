package org.csu.restaurant.restaurantserver.dto;
import lombok.AllArgsConstructor;
import lombok.Data;
@Data
@AllArgsConstructor
public class RewardResultDTO { private Integer rewardId; private String rewardName; private Integer remainingChances; }
