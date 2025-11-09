import iio
import numpy as np
import struct
import json

def data_collect():
    ctx = iio.Context("ip:192.168.10.131")
    adc0 = ctx.find_device("cf_axi_adc")
    adc1 = ctx.find_device("cf_axi_adc_1")


    # 配置需要打开的通道
    adc0_enables=[0,1,2,3,4,5,6,7]
    for i in adc0_enables:
        adc0.channels[i].enabled = True
    adc1_enables=[0,1,2,3,4]
    for i in adc1_enables:
        adc1.channels[i].enabled = True
    # 单次采样点, 采集次数, scale常量值
    points = 2560
    rounds = 5
    allPoints = points*rounds
    scale = float(adc0.channels[0].attrs['scale'].value)

    buf0 = iio.Buffer(adc0, points, cyclic=False)
    buf1 = iio.Buffer(adc1, points, cyclic=False)
    while True:

        # 考虑同步需求, 少量多批次获取两个AD的数据
        adc0_bytes = bytearray()
        adc1_bytes = bytearray()
        for i in range(rounds):
            buf0.refill()
            adc0_bytes.extend(buf0.read())
            buf1.refill()
            adc1_bytes.extend(buf1.read())

        # 解析格式: 小端, 总个数, 类型为32位无符号
        unpackFormat = '<' + str(allPoints*len(adc0_enables)) + 'I'
        adc0_data = np.array(struct.unpack(unpackFormat, adc0_bytes))
        adc0_data = adc0_data.reshape(allPoints, len(adc0_enables))

        unpackFormat = '<' + str(allPoints*len(adc1_enables)) + 'I'
        adc1_data = np.array(struct.unpack(unpackFormat, adc1_bytes))
        adc1_data = adc1_data.reshape(allPoints, len(adc1_enables))

        # 进行列合并
        adc_data = np.concatenate((adc0_data, adc1_data), 1)
        adc_data = np.int32(adc_data<<8)
        adc_data = adc_data>>8
        adc_data = adc_data*scale

       # json_path_name = '/rtfdata/third_party/datafile/adc_data.json'
        json_path_name = '/rtfdata/third_party/datafile/adc_data.json'#可以将数据存放到哪个盘的路径
        dict = {}
        for chan in range(len(adc0_enables)+len(adc1_enables)):
            dict[str(chan)] = (adc_data[:, chan]).tolist()

        with open(json_path_name, 'w') as f1:
            f1.write(json.dumps(dict, indent=2, ensure_ascii=False))
        return

data_collect()