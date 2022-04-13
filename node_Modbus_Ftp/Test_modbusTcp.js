var ModbusRTU = require("modbus-serial");
var client = new ModbusRTU();
var index = 0;

if(client.isOpen){
    schedule();
}else{
    client.connectTCP("10.63.1.31",{port:502}).then(() =>{
        console.log(client.isOpen)
        client.setID(0);
        schedule();
    
        // // 寫入 4x003 值70
        // client.writeRegister(2,70).then(()=>{
    
        //     // 位址範圍僅支援 4x001 ~ 4x111
        //     // 讀取 4x001 ~ 4x070
        //     client.readHoldingRegisters(0,70,function(err,data){
        //         console.log(data)
        //         console.log(err)
        //     })
        // })
    })
}
function schedule (){
    setInterval(() => {
        client.readHoldingRegisters(0 + index * 10,10,function(err,data){
            if(err){
                console.log('err '+ err)
            }else{
                switch(data.data[0])
                {
                    // 未知
                    case 0:
                        break;
                    // 寫入
                    case 1:
                        break;
                    // 完成
                    case 2:
                        console.log(' 開始 : ' + data.data[1] + ':' + data.data[2] + ':' + data.data[3] + ',' +
                                    ' 結束 : ' + data.data[4] + ':' + data.data[5] + ':' + data.data[6] + ',' +
                                    ' 工一 : ' + data.data[7] + ',' +
                                    ' 工二 : ' + data.data[8] + ',' +
                                    ' 工三 : ' + data.data[9] )

                        client.writeRegister(0 + index * 10, 4)
                        break;
                    // 刪除
                    case 4:
                        break;
                }   
                index ++;
                if(index > 1)
                    index = 0;
            }
        })
        

    },1200000)
}




