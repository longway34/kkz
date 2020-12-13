/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */



// Ачинский набор данных
var GR_POSITION_EVENT           =   3;  //3 - значение event bar
var GR_POSITION_HREADY          =   4;  //4 - значение sstate bar
var GR_POSITION_SREADY          =   5;  //5 - значение hstate
var GR_POSITION_WARNING         =   6;  //6 - значение warning
var GR_POSITION_TIME            =   7;  //7 - time
var GR_POSITION_EV_MASK         =   9;
var GR_POSITION_SQ_MIN         =   10;

var grMaxGraphicCount = 11;    // 0 - 2 - значения кривых: 0 - сигнал в сигмах; 1 - максимальное сигма 2 - sigma
                            // 3 - значение event bar
                            // 4 - значение sstate bar
                            // 5 - значение hstate
                            // 6 - значение warning;
                            // 7 - time
                            // 8 - diff signal
                            // 9 - ev_mask
var grMaxLength = 500;
var sourcesMaxGraphicCount = 27; // 0 - 17 - data
                                // 18 - 23 - events
                                // 24 - time
                                // 19 - mark
                                // 25 - evMask

var pos_signal_sigma = [0, 3, 6, 9, 12, 15];
var pos_sq     = [1, 4, 7, 10, 13, 16];
var pos_sigma  = [2, 5, 8, 11, 14, 17];
var pos_events = [18, 19, 20, 21, 22, 23];
var pos_time = 24;
var pos_mark = 25;
var pos_evMask = 26;

var event_mask_current;

var sourcesData = new Array(grMaxLength);
var grData = new Array(grMaxGraphicCount);
var currentLastTime =0;
var currentGrLength =0;

function graphicsClear(){
    for(var gr=0; gr<grMaxGraphicCount; gr++){
    //    grData[gr] = new Array(grMaxLength);
        grData[gr] = [];
        for(var i=0; i<grMaxLength; i++){
            grData[gr][i] = [i, null];
        }
    }
    sourcesData = new Array(grMaxLength);
    currentLastTime =0;
    currentGrLength =0;
}

graphicsClear();

function getMax(arg){
    if($.isArray(arg)){
        var max = arg[0];
        for(var i=1; i< $(arg).length; i++){
            if(max < arg[i]) max = arg[i];
        }
        return max;
    } else {
        return arg;
    }
}
function getMin_(arg){
    if($.isArray(arg)){
        var min = 10000;
        if(arg < 0) arg = 0;
        for(var i=1; i< $(arg).length; i++){
            if(min > arg[i] && arg[i] > 0) min = arg[i];
        }
        return min === 10000 ? -1 : min;
    } else {
        return arg;
    }
}

function getMin(arg){
    if($.isArray(arg)){
        var min = arg[0];
        for(var i=1; i< $(arg).length; i++){
            if(min > arg[i]) min = arg[i];
        }
        return min;
    } else {
        return arg;
    }
}
var MinMax = function(){
    this.min = 1e6;
    this.max = -1e6;
    this.clear = function(){
        this.min = 1e6;
        this.max = -1e6;
    };
};

var min_max = new MinMax();
var min_max_1 = new MinMax();


function getParseEvMask(evMask, channels){
    var ret = 0; 
    $.each(channelsLists[channels], function(index, self){
        var bit = evMask & (1 << self);
        if(bit !== 0){
            ret |= (1 << self);
        }
    });
    return ret;
};

function dataArraySet(index, shift, len, channel, values, pSigma){ // index - текущий индекс;
                                                                 // shift - сдвиг
                                                                 // len - длина начального массива
                                                                 // channel : -3 ~ 5 // show select channel
                                                                 // values = array[grMaxGraphicCount]
                                                                 // pSigma - пороговое значение сигма ([6] vControls.getSigma())
    if(index < shift){
        return;
    }
    if($(values).length === 0){
        return;
    }
    var index = index - shift;
    
    if(index+shift < len){  // старые данные - нужно сдвинуть пересчитать пороги
        sourcesData[index] = sourcesData[index+shift];
        for(var gr=0; gr<grMaxGraphicCount; gr++){
            if(grData[gr][index+shift] === null){
                grData[gr][index] = null;
            } else {
                grData[gr][index] = [index, grData[gr][index+shift][1]];
            }
        }
    } else {                // новый набор берем из values
        sourcesData[index] = values;
        var d0 = [], d1 = [], d8 = []; 
        for(var ch=0; ch<6; ch++){
//            d0[ch] = Math.abs(sourcesData[index][pos_mo[ch]] - sourcesData[index][pos_avr[ch]]) / sourcesData[index][pos_sigma[ch]];
            d0[ch] = sourcesData[index][pos_signal_sigma[ch]];
            d1[ch] = sourcesData[index][pos_sigma[ch]];
            d8[ch] = sourcesData[index][pos_sq[ch]];
        }
        switch(channel){
            case -3: // дальняя половина
                grData[0][index] = [index, getMax([d0[3], d0[4], d0[5]])];
                grData[2][index] = [index, getMax([d1[3], d1[4], d1[5]])];
                grData[8][index] = [index, getMin_([d8[3], d8[4], d8[5]])];
                break;
            case -2: // ближняя половина
                grData[0][index] = [index, getMax([d0[0], d0[1], d0[2]])];
                grData[2][index] = [index, getMax([d1[0], d1[1], d1[2]])];
                grData[8][index] = [index, getMin_([d8[0], d8[1], d8[2]])];
                break;
            case -1: // все каналы
                grData[0][index] = [index, getMax(d0)];
                grData[2][index] = [index, getMax(d1)];
                grData[8][index] = [index, getMin_(d8)];
                break;
            default:
                grData[0][index] = [index, d0[channel]];
                grData[2][index] = [index, d1[channel]];
                grData[8][index] = [index, getMin_([0, d8[channel]])];
                break;
        };
// заполняем events sstate hstate
        var mmm = getParseEvMask(sourcesData[index][pos_evMask], channel);
        if(sourcesData[index][pos_mark] & 0xF0 !== 0){
            grData[GR_POSITION_WARNING][index] = [index, 5];
        } else {
            grData[GR_POSITION_WARNING][index] = [index, null];
        }
        if(getParseEvMask(sourcesData[index][pos_evMask], channel) !== 0){
            grData[GR_POSITION_EVENT][index] = [index, 100];
        } else {
            grData[GR_POSITION_EVENT][index] = [index, null];
        }
        if(sourcesData[index][pos_mark] & 1 === 1){
            grData[GR_POSITION_SREADY][index] = [index, 15];
        } else {
            grData[GR_POSITION_SREADY][index] = [index, null];
        }
        if(sourcesData[index][pos_mark] & 2 === 2){
            grData[GR_POSITION_HREADY][index] = [index, 10];
        } else {
            grData[GR_POSITION_HREADY][index] = null;
        }
        grData[GR_POSITION_EV_MASK][index] = [index, sourcesData[index][pos_evMask]];
        event_mask_current |= grData[GR_POSITION_EV_MASK][index][1];
    }
    grData[1][index] = [index, pSigma];
    grData[GR_POSITION_SQ_MIN][index] = [index, sqMin];
//    grData[2][index] = null;
    grData[GR_POSITION_TIME][index] = [index, sourcesData[index][pos_time]];
    if(sourcesData[index][pos_time] > currentLastTime) currentLastTime = sourcesData[index][pos_time];
    if(!isFinite(grData[0][index][1])){
        grData[0][index][1]=0;
    }
    if(min_max.min > grData[0][index][1]){ min_max.min = grData[0][index][1];}
    if(min_max.max < grData[0][index][1]){ min_max.max = grData[0][index][1];}
    
    event_mask_current |= grData[GR_POSITION_EV_MASK][index][1]; 
    
//    min_max_1.min = getMin([min_max_1.min, grData[2][index][1], grData[8][index][1]]);
//    min_max_1.min = -0.2;
//    if(min_max_1.max < grData[8][index][1]) { min_max_1.max = grData[8][index][1];}
//    if(min_max_1.min > grData[8][index][1]) { min_max_1.min = grData[8][index][1];}
//    min_max_1.max = getMax([min_max_1.max, grData[2][index][1], grData[8][index][1]]);
//    min_max_1.max = getMax([min_max_1.max, 0.5, grData[8][index][1]]);
//    min_max_1.max = 0.5;
      min_max_1.min = 0;
      min_max_1.max = 0.2;
};

function fullData(xml, channel, pSigma){
    $(xml).find("data").each(function(){
        var len = parseInt($(this).attr("len"));
        var shift = 0;
        var newLen = currentGrLength + len; 

        min_max.clear();
        min_max.max = pSigma;
    
        if(newLen >= grMaxLength){
            shift = len;
            newLen = grMaxLength;
        }
        event_mask_current = 0;
// смещаем текущие данные        
        var dstIndex = 0; var srcIndex = shift;
        $(sourcesData).each(function(index, sourse){
            dataArraySet(index, shift, currentGrLength, channel, sourse, pSigma);
            if(index > shift) event_mask_current |= grData[GR_POSITION_EV_MASK][index][1];
        });
        
        
        $(this).find("d").each(function(){
            var source = [];
            $.each($(this).text().split(" "), function(index, data){
                if(index < 18){
                    source[index] = parseFloat(data);
                } else {
                    source[index] = parseInt(data);
                }
            });
            var xmlIndex = parseInt($(this).attr("x"));
            dataArraySet(xmlIndex+currentGrLength,shift,currentGrLength,channel,source,pSigma);
        });
        currentGrLength = newLen;
        
    });
    var border = (min_max.max - min_max.min) * 0.1;
    min_max.min = min_max.min - border;
    min_max.max = min_max.max + border;
};

var H_READY_COLOR   =  "#6DCBFF";
var S_READY_COLOR   =  "#FFCC99";
var FIRST_GR_COLOR  =  "#FF0000";
var SECOND_GR_COLOR =  "#6DCBFF";
var P1_COLOR        =  FIRST_GR_COLOR;
var P2_COLOR        = "00FF00";
var EVENTS_COLOR    = "#00C500";
var SQ_MIN_COLOR    = "#808040";

function paint(canvas, channels){
    data_ = [];
    switch(addSigmaData.getTypeGraphic()){
        case 0:
        data_ = [
//            {data: grData[GR_POSITION_SREADY], yaxis: 2, color: S_READY_COLOR, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
//            {data: grData[GR_POSITION_HREADY], yaxis: 2, color: H_READY_COLOR, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
            {data: grData[GR_POSITION_EVENT], yaxis: 2, label: "Metall", color: EVENTS_COLOR, bars: {show: true, lineWidth: 1, barWidth: 1, fill: true}},
//            {data: grData[GR_POSITION_WARNING], yaxis: 2, color: FIRST_GR_COLOR, bars: {show: true, lineWidth: 1, barWidth: 1, fill: true}},

            {data: grData[0], label: "Критерий ch: "+channels, color: FIRST_GR_COLOR  },
            {data: grData[1], label: "Порог", color: SECOND_GR_COLOR},

//            {data: grData[2], yaxis: 2, label: "Sigma ch: "+channels},
        ];
        break;
        case 1:
        data_ = [
            {data: grData[GR_POSITION_EVENT], yaxis: 2, label: "Metall", color: EVENTS_COLOR, bars: {show: true, lineWidth: 1, barWidth: 1, fill: true}},
            {data: grData[0], label: "Отношения ch: "+channels, color: FIRST_GR_COLOR  },
            {data: grData[1], label: "МО", color: SECOND_GR_COLOR},
        ];            
    }
        data_add = [];
        if(vControls.isAddGraphics()){
          data_add = [
            {data: grData[8], /*yaxis: 2, */label: "SQ ch:"+channels},
            {data: grData[GR_POSITION_SQ_MIN], yaxis: 2, label: "SQ max", color: SQ_MIN_COLOR}  
          ];  
            data_.push(data_add[0], data_add[1]);
        }
    $.plot(canvas,
        data_, {
            legend: {
                position: 'nw'
            },
            y2axis: {
                color: '#C0D800',
                min: min_max_1.min,
                max: min_max_1.max
            },
            yaxis: {
                color: '#000080',
                min: min_max.min,
                max: min_max.max
            },
            series: {
                lines: {
                    show: true,
                    lineWidth :1
                }
            }
        }
    );
    
    
}
