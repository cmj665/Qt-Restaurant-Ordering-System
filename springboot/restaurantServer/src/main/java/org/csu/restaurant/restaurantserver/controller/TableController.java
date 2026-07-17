package org.csu.restaurant.restaurantserver.controller;

import org.csu.restaurant.restaurantserver.entity.DiningTable;
import org.csu.restaurant.restaurantserver.dto.TableStatusDTO;
import org.csu.restaurant.restaurantserver.service.TableService;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/table")
@RequiredArgsConstructor
public class TableController {
    private final TableService tableService;

    //查询所有桌台
    @GetMapping("/list")
    public List<DiningTable> list() {
        return tableService.findAll();
    }

    //修改桌台状态
    @PostMapping("/status")
    public String updateStatus(@RequestBody TableStatusDTO request) {
        tableService.changeStatus(request.getTableId(), request.getStatus());
        return "修改成功";
    }

    //清理桌台
    @PostMapping("/clean/{id}")
    public String clean(@PathVariable Integer id){
        tableService.clean(id);
        return "清台成功";
    }

}
