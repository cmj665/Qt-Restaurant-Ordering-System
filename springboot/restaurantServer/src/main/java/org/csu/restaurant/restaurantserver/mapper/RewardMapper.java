package org.csu.restaurant.restaurantserver.mapper;
import java.util.List;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.csu.restaurant.restaurantserver.entity.Order;
import org.csu.restaurant.restaurantserver.entity.Reward;
@Mapper
public interface RewardMapper {

    List<Reward> findAll();

    //悲观锁锁定未支付订单，防止并发多人同时抽奖超次数
    Order lockUnpaidOrder(Integer orderId);

    //统计该订单已经抽过多少次奖
    int countDraws(Integer orderId);

    //插入中奖记录
    int insertOrderReward(@Param("orderId")Integer orderId,@Param("rewardId")Integer rewardId);
}
