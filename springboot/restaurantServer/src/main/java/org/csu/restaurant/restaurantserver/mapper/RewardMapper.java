package org.csu.restaurant.restaurantserver.mapper;
import java.util.List;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.csu.restaurant.restaurantserver.entity.Order;
import org.csu.restaurant.restaurantserver.entity.Reward;
@Mapper
public interface RewardMapper {
    List<Reward> findAll();
    Order lockUnpaidOrder(Integer orderId);
    int countDraws(Integer orderId);
    int insertOrderReward(@Param("orderId")Integer orderId,@Param("rewardId")Integer rewardId);
}
