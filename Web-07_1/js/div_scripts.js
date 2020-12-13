/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

//var URL_ = document.baseURI;
var URL_ = "http://192.168.88.222:8000/div_index.html";
//var URL_ = "http://192.168.223.101:8000/div_index.html";
var stateControlPanels;

var currentTTL = 1000;

var needChangeAllData = true;
var ZONES_NUMBER    = 5;

var TYPE_DATA_DMA               =   0; // исходный ДМА сигнал
var TYPE_DATA_SUMMS             =   1; // полученные суммы

var TYPE_DATA_RATIOS            =   3; // полученные отношения
var TYPE_DATA_AVR_RATIOS        =   2; // среднее значение отношений

var TYPE_DATA_DIFF_RATIO        =   11; // разность ratios100[channel][s2/s1]-ratios100[channel][s1/s0]

var TYPE_DATA_DIFF              =   16; // среднеквадратичное отклонение от % отношений
var TYPE_DATA_DIFF2             =   17; // среднеквадратичное отклонение от % отношений со знаком
var TYPE_DATA_CRITERY           =   18; // график поведения критерия от 09.12.2014
var TYPE_DATA_CRITERY2          =   19; // график поведения критерия от 09.12.2014

var TYPE_DATA_CORREL            =   20;  // график межканальной корреляции от 23.04.2016
var TYPE_DATA_DIFF_ALL_CHANNELS =   21;  // график дисперсии отношений

var TYPE_DATA_F2P_SUMMS         =   22;  // график отфильтрованных сумм
var TYPE_DATA_3_SIGMA           =   23;  // график 3 среднеквадратичных отклонений

var TYPE_NO_DATA                =   100;

var H_READY_COLOR   =  "#6DCBFF";
var S_READY_COLOR   =  "#FFCC99";
var FIRST_GR_COLOR  =  "#FF0000";
var SECOND_GR_COLOR =  "#6DCBFF";
var P1_COLOR        =  FIRST_GR_COLOR;
var P2_COLOR        = "00FF00";
var EVENTS_COLOR    = "#00C500";

var grPanels = new Array(4); // array of graphic panels
var timeOutRefresh = 2000;
//grPanels[0] = grObject(0, null);
//grPanels[1] = grObject(1, null);
//grPanels[2] = grObject(2, null);
//grPanels[3] = grObject(3, null);
var panelInFocus = -1;

var imLamps = {
    red: new Image(),
    white: new Image()
};

//imLamps_red = new Image();
//imLamps_red.src = "images/lamp_ellow.png";
//
//imLamps_white = new Image();
//imLamps_white.src = "images/lamp_white_3.png";

lampsPanel = function(){
    this.lamps = new Array(6);
    this.images = {
        red: new Image(),
        white: new Image()
    }
    
    this.images.red.src = "images/lamp_ellow.png";
    this.images.white.src = "images/lamp_white_3.png";
    
    for(var i=0; i < 6; i++){
            var name = "#l_"+i;
            this.lamps[i] = $(name)[0];
//            alert("create "+i+" as "+this.lamps[i]);
    }
    
    this.ligth = function(code){
        for(var i=0; i<6; i++){
            var mask = 1 << i;
            if((code & mask) === mask){
                this.lamps[i].src = imLamps_red.src;
//                $(this.lamps[i]).removeClass("lamps_white");
//                $(this.lamps[i]).addClass("lamps_yellow");
//                this.lamps[i].src = imLamps.red.src; //getContext("2d").drawImage(imLamps.red);
//                this.lamps[i] = imLamps.red; //getContext("2d").drawImage(imLamps.red);
            } else {
                this.lamps[i].src = imLamps_white.src;
//                $(this.lamps[i]).addClass("lamps_yellow");
//                $(this.lamps[i]).removeClass("lamps_white");
//                this.lamps[i].src = imLamps.white.src; //getContext("2d").drawImage(imLamps.white);
//                this.lamps[i] = imLamps.white; //getContext("2d").drawImage(imLamps.white);
            }
        }
    };
};

var lp = 0;

var mesurer = function(f){ 
    var ts, te; ts = new Date(); 
    f(); 
    te = new Date(); 
    return te.getTime() - ts.getTime(); 
};

var lp;

function LastMyTime(_lt){
    this.lastTime = parseInt(_lt);
    this.getLastTime = function(){
        return this.lastTime;
    };
    this.setLastTime = function(_lt){
        this.lastTime = parseInt(_lt);
    };
};

var lastMyTime;
var iAmPaint = false;

/********************** One Graphic panel object ********************/
function grObject(_index, _parent){
    this.index = _index;
    this.parent = _parent;
    this.name = "Graphic "+_index;
    
    
    this.canvas = $("#gr_canvas_"+this.index).first();
    
    this.channel = $("#gr_channel_"+this.index);
    this.type = $("#gr_type_"+this.index);
    this.kk = $("#gr_kk_"+this.index);
    this.porog1 = $("#gr_porog_k1_"+this.index);
    this.porog2 = $("#gr_porog_k2_"+this.index);
    this.correl = $("#gr_porog_corr_"+this.index);
    this.corrLabel = $("#corrLabel_"+this.index);
    this.ratio  = $("#gr_ratio_"+this.index);       

    this.lTime = new LastMyTime("0");
    
    this.storePicture = function(fName){
        if(isNaN(fName) || fName === ""){
            fName = "storePicture_"+this.index+".png";
        }
        var cnvs = $(this.canvas).children("canvas");
        cnvs[0].toBlobHD(function(blob){
            saveAs(blob, fName);
        });
    }
    
    this.getLastTime = function(){
        return this.lTime.getLastTime();
    };
    this.setLastTime = function(lt){
        this.lTime.setLastTime(lt);
    };
    
    this.clearGraphicData = function(){
        for(var i=0; i<8; i++){
            all_data.data[i] = new Array();
        }
//        all_data.data[_index*2] = new Array();
//        all_data.data[_index*2 + 1] = new Array();
        all_data.lastTime = 0;
        all_data.len = 0;
    };
    this.zones = new Array(ZONES_NUMBER);
    for(var z=0; z<ZONES_NUMBER; z++){
        this.zones[z] = {
            min:  $("#z_"+this.index+"_"+z+"_min"),
            max:  $("#z_"+this.index+"_"+z+"_max"),
            getMin:  function(){
                var ret = parseInt($(this.min).val());
                return ret;
            },
            getMax:  function(){
                var ret = parseInt($(this.max).val());
                return ret;
            },
            set:    function(min, max){
                $(this.min).val(min);
                $(this.max).val(max);
            }
        };
    }
    this.allInputs = $().add(this.kk).add(this.porog1).add(this.porog2).add(this.correl);
    for(var z=0; z < ZONES_NUMBER; z++){
//        this.allInputs[this.allInputs.length] = this.zones[z].min;
//        this.allInputs[this.allInputs.length] = this.zones[z].max;
          this.allInputs = $(this.allInputs).add(this.zones[z].min).add(this.zones[z].max);
    }
    
    $(this.allInputs).each(function(){
        $(this).focusin(function(e){
           panelInFocus = _index; 
        });
        
    });
    
    this.setCalibrateToServer = function(){
        var kk = this.getKK(); var p1 = this.getPorog1(); var p2 = this.getPorog2();
        var pcorr = this.getCorrel(); var ch = this.getChannel();
        var queryStr = URL_+"?command=set_calibrate&channel="+ch+"&kk="+kk+"&p1="+p1+"&p2="+p2+"&pcorr="+pcorr;
        $.ajax({
            url: queryStr,
            type: "GET",
            typeData: "xml",
            success: function(xml){
                $(xml).find("status").each(function(){
                    if($(this).text().trim().toUpperCase() === "OK"){
//                        alert("Update Calibrate for channel #"+grPanels[_index].getChannel()+" success...");
                    } else {
                        alert("Update Calibrate for channel #"+grPanels[_index].getChannel()+" fault...");
                    }
                });
            },
            errors:function(xml){
                    alert("Update Calibrate for channel #"+grPanels[_index].getChannel()+" fault...");
            }
        });
    };

    this.getChannel = function(){
        var ret = parseInt($(this.channel).val());
        return ret;
    };
    this.channel.change(function(){
       needChangeAllData = true;
       grPanels[_index].clearGraphicData();
    });
    
    this.setZones = function(name){
       var query = "?command=set_zone&ch="+this.getChannel();
       var z = parseInt(name);
          query += "&z="+z+"&min="+this.zones[z].getMin()+"&max="+this.zones[z].getMax(); 
       $.ajax({
           url: URL_ + query,
           type: "GET",
           typeData: "xml",
           success: function(xml){
                $(xml).find("status").each(function(){
                    if($(this).text().trim().toUpperCase() === "OK"){
                        alert("Update Zones for channel #"+grPanels[_index].getChannel()+" success...");
                    } else {
                        alert("Update Zones for channel #"+grPanels[_index].getChannel()+" fault...");
                    }
                });
           }
       });
    };
    
    for(var z=0; z < ZONES_NUMBER; z++){
//        this.zones[z] = {min : $("#z_"+this.index+"_"+z+"_min").first(),
//                         max:  $("#z_"+this.index+"_"+z+"_max").first()
//        };
//        this.zones[z].getMin = function(){
//            var ret = parseInt($(this.zones[z].min).val());
//            return ret;
//        };
//        this.zones[z].getMax = function(){
//            var ret = parseInt($(this.zones[z].max).val());
//            return ret;
//        }
        
        this.zones[z].min.change(function(e){
            grPanels[_index].setZones(e.target.name);
        });
        this.zones[z].max.change(function(e){
            grPanels[_index].setZones(e.target.name);
        });
    };
    this.getType = function(){
        var ret = parseInt($(this.type).val());
        return ret;
    };
    this.type.change(function(e){
       needChangeAllData = true;
       grPanels[_index].clearGraphicData();
       switch(grPanels[_index].getType()){
           case TYPE_DATA_DMA:
           case TYPE_DATA_DIFF2:
           case TYPE_DATA_CRITERY:
               $(grPanels[_index].ratio).addClass("gr_control_element_hidden");
               $(grPanels[_index].correl).removeClass("gr_control_element_hidden");
               $(grPanels[_index].corrLabel).removeClass("gr_control_element_hidden");
               break;
           case TYPE_DATA_SUMMS:
           case TYPE_DATA_F2P_SUMMS:
               $(grPanels[_index].ratio).html('<option disabled value="-1">Выб. суммы</option>'
                        +'<option value="0">Сумма 0</option>'
                        +'<option value="1" selected>Сумма 1</option>'
                        +'<option value="2">Сумма 2</option>');

               $(grPanels[_index].ratio).prop("title", "Выбор суммы для графика");
               $(grPanels[_index].ratio).removeClass("gr_control_element_hidden");
               $(grPanels[_index].correl).addClass("gr_control_element_hidden");
               $(grPanels[_index].corrLabel).addClass("gr_control_element_hidden");
               break;
           case TYPE_DATA_RATIOS:
           case TYPE_DATA_DIFF:
               $(grPanels[_index].ratio).html('<option disabled value="-1">Выб. отнош.</option>'
                        +'<option value="0" selected>Отнош. 0</option>'
                        +'<option value="1">Отнош. 1</option>');

               $(grPanels[_index].ratio).prop("title", "Выбор отношения для графика");
               $(grPanels[_index].ratio).removeClass("gr_control_element_hidden");
               $(grPanels[_index].correl).addClass("gr_control_element_hidden");
               $(grPanels[_index].corrLabel).addClass("gr_control_element_hidden");
               break;
       }; 
    });
    
    this.getRatio = function(){
        var ret = parseInt($(this.ratio).val());
        return isNaN(ret) ? 0 : ret;
    };
    this.ratio.change(function(e){
       needChangeAllData = true;
       grPanels[_index].clearGraphicData();
    });
    
    this.getKK = function(){
        var ret = parseFloat($(this.kk).val());
        return ret;
    };
    this.setKK = function(_kk){
        $(this.kk).val(_kk);
    };
    this.kk.bind("change",function(e){
        grPanels[_index].setCalibrateToServer();
    
    });
    
    
    this.getPorog1 = function(){
        var ret = parseFloat($(this.porog1).val());
        return ret;
    };
    this.setPorog1 = function(_porog){
        $(this.porog1).val(_porog);
    };
    this.porog1.bind("change", function(e){
        grPanels[_index].setCalibrateToServer();
    });

    this.getPorog2 = function(){
        var ret = parseFloat($(this.porog2).val());
        return ret;
    };
    this.setPorog2 = function(_porog){
        $(this.porog2).val(_porog);
    };
    this.porog2.bind("change", function(e){
        grPanels[_index].setCalibrateToServer();

    });
    
    this.getCorrel = function(){
        var ret = parseFloat($(this.correl).val());
        return ret;
    };
    this.setCorrel = function(_correl){
        $(this.correl).val(_correl);
    };
    this.correl.bind("change",function(e){
        grPanels[_index].setCalibrateToServer();
    });
    
    this.isReadyToChange = function(){
        var res = $(this.allInputs).find(":focus").size();
        if(res > 0){
            return false;
        } else {
            return true;
        }
    };
//*********    
    this.setParams = function(xml){
        var channel = this.getChannel();
        $(xml).find("cdata").each(function(){
            
            var ch = parseInt($(this).attr("ch"));
            if(ch === channel){
                if(grPanels[_index].isReadyToChange()){
                    grPanels[_index].setKK($(this).attr("kk"));
                    grPanels[_index].setPorog1($(this).attr("pkk01"));
                    grPanels[_index].setPorog2($(this).attr("pkk02"));
                    grPanels[_index].setCorrel($(this).attr("pcorr01"));
                }
            }
        });
    };
    this.setXMLZones = function(xml){
        var channel = this.getChannel();
        $(xml).find("zones").each(function(){
            var ch = parseInt($(this).attr("ch"));
            if(ch === channel){
                $(this).find("z").each(function(){
                    var zn = parseInt($(this).attr("num"));
                    var min = parseInt($(this).attr("min"));
                    var max = parseInt($(this).attr("max"));
                    
                    grPanels[_index].zones[zn].set(min, max);
                })
            }
        })
    }

    this.data = new Array(grMaxGraphicCount);
    this.grLength = 0; // Current length array of grapics data
    
    this.isEvents = new Array();
    this.hReady = new Array();
    this.sReady = new Array();
    this.p1 = new Array();
    this.p2 = new Array();
    
    for(var i=0; i<grMaxGraphicCount; i++){
        this.data[i] = new Array(grMaxLength);
    }
    
    this.xmlIndex = _index * 2;
    this.count = 0;
    this.min3;
    this.max3;
    
    this.fullData = function(xml){
        var ind0 = _index * 3;
        var ind1 = _index * 3 + 1;
        var ind2 = _index * 3 + 2;
        this.min3 = 1e6; this.max3 = -1e6;
        for(var index in all_data.data[ind0]){
            var grt = all_data.data[ind0][index][0];
// Определим размеры по вертикали для событий, готовностей
            var bars_value;
            if(Math.abs(all_data.min_max[ind0].max) > Math.abs(all_data.min_max[ind0].min))
                bars_value = all_data.min_max[ind0].max;
            else
                bars_value = all_data.min_max[ind0].min;
            
            if(all_data.types[ind0] !== 0){ // не ДМА график - заполняем events и пороги
// События
                if(all_data.data[GR_POSITION_EVENT][index] !==  null){
                    var ev = all_data.data[GR_POSITION_EVENT][index];
                    if((ev[1] & (1 << this.getChannel())) !== 0){
                        this.isEvents[index] = [grt, bars_value];
                    } else {
                        this.isEvents[index] = null;
                    }
                } else {
                    this.isEvents[index] = null;
                }
// Готовность оборудования
                if(all_data.data[GR_POSITION_HREADY][index] !==  null){
                    
                    if(all_data.data[GR_POSITION_HREADY][index][1] !== 0){
                        this.hReady[index] = [grt, bars_value];
                    } else {
                        this.hReady[index] = null;
                    }
                } else {
                    this.hReady[index] = null;
                }
// Готовность софта (паузы по нашему)
                if(all_data.data[GR_POSITION_SREADY][index] !==  null){
                    if(all_data.data[GR_POSITION_SREADY][index][1] !== 0){
                        this.sReady[index] = [grt, bars_value];
                    } else {
                        this.sReady[index] = null;
                    }
                } else {
                    this.hReady[index] = null;
                }
// Пороги
                if(this.getType() === TYPE_DATA_CRITERY2){
                    this.p1[index] = [grt, $(this.porog1).val()];
                    this.p2[index] = [grt, $(this.correl).val()];
                }
                if(this.getType() === TYPE_DATA_DIFF2){
                    this.p1[index] = [grt, all_data.data[ind1][index][1] + all_data.data[ind2][index][1] * $(this.kk).val()];
                    this.p2[index] = [grt, all_data.data[ind1][index][1] - all_data.data[ind2][index][1] * $(this.kk).val()];
                    if(this.min3 > this.p2[index][1]) this.min3 = this.p2[index][1];
                    if(this.max3 < this.p1[index][1]) this.max3 = this.p1[index][1];
                }
                if(this.getType() === TYPE_DATA_F2P_SUMMS){
//                    all_data.data[ind1][index][1] = all_data.data[ind1][index][1] * $(this.porog1).val();
                }
            };
        }
    };
//    };
    this.paint = function(){
        var data_ = [];
        var ind0 = _index * 3; var ind1 = _index * 3 + 1; var ind2 = _index * 3 +2;
        switch(this.getType()){
//            case TYPTYPE_DATA_DMA: // DMA
//                data_ = [ 
//                    {data: all_data.data[ind0], label: "DMA Source ch: "+this.getChannel() }
//                ];
//                break;
            case TYPE_DATA_CRITERY: // Критерий
                data_ = [
                    {data: this.sReady, color: S_READY_COLOR, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
                    {data: this.hReady, color: H_READY_COLOR /*ligth blue*/, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
                    {data: this.isEvents, label: "Metall", color: EVENTS_COLOR,bars: {show: true, lineWidth: 1, barWidth: 1, 
                    fill: true}},
                    
                    {data: all_data.data[ind0], label: "Критерий ch: "+this.getChannel(), color: FIRST_GR_COLOR  },
                    {data: all_data.data[ind1], label: "Корреляция", yaxis: 2, color: SECOND_GR_COLOR},
//                    {data: this.data[8], label: "Среднее.", color: "#A91181"},
//                    {data: this.data[9], color: "#A91181"},
                    {data: this.p1, label: "Порог критерия", color: P1_COLOR},
                    {data: this.p2, label: "Порог критерия 2", yaxis: 2, color: P2_COLOR}
                ];
                break;
            case TYPE_DATA_DIFF: // 16 Критерий 2 Ачинск
                data_ = [
                    {data: this.sReady, color: S_READY_COLOR, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
                    {data: this.hReady, color: H_READY_COLOR /*ligth blue*/, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
                    {data: this.isEvents, label: "Metall", color: EVENTS_COLOR,bars: {show: true, lineWidth: 1, barWidth: 1, 
                    fill: true}},
                    
                    {data: all_data.data[ind0], label: "Отношение 0 ch: "+this.getChannel(), color: FIRST_GR_COLOR  },
                    {data: all_data.data[ind1], label: "3 Сигмы", color: SECOND_GR_COLOR},
                    {data: all_data.data[ind2], label: "Разность", color: EVENTS_COLOR, yaxis: 2}
//                    {data: this.data[8], label: "Среднее.", color: "#A91181"},
//                    {data: this.data[9], color: "#A91181"},
//                    {data: this.p1, label: "Порог критерия", color: P1_COLOR},
//                    {data: this.p2, label: "Порог критерия 2", yaxis: 2, color: P2_COLOR}
                ];
                break;
            case TYPE_DATA_DIFF2: // Настройки 23.06.2016
                data_ = [
                    {data: this.sReady, color: S_READY_COLOR, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
                    {data: this.hReady, color: H_READY_COLOR /*ligth blue*/, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
                    {data: this.isEvents, label: "Metall", color: EVENTS_COLOR,bars: {show: true, lineWidth: 1, barWidth: 1, 
                    fill: true}},
                    {data: all_data.data[ind0], label: "Критерий: ch"+this.getChannel(), color: FIRST_GR_COLOR },
                    {data: this.p1, color: P2_COLOR, label: "Пороги:"},
                    {data: this.p2, color: P2_COLOR}
                ];
                
                break;
//            case 16: // SCO 16
//                data_ = [
//                    {data: this.sReady, color: S_READY_COLOR, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
//                    {data: this.hReady, color: H_READY_COLOR, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
//                    {data: this.isEvents, label: "Metall", color: EVENTS_COLOR,bars: {show: true, lineWidth: 1, barWidth: 1, 
//                    fill: true }},
//                    {data: all_data.data[ind0], label: "Summander: ch"+this.getChannel(), color: FIRST_GR_COLOR },
////                    {data: all_data.data[ind1], label: "Дисперсия:", yaxis: 2, color: SECOND_GR_COLOR}
//                ];
//                break;
            case TYPE_DATA_SUMMS: // Суммы
                data_ = [{data: this.isEvents, color: EVENTS_COLOR, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
                    {data: all_data.data[ind0], label: "Сумма ch: "+this.getChannel() }
                ];
                break;
            case TYPE_DATA_RATIOS: // 3 Отношения
                data_ = [
                    {data: this.isEvents, color: EVENTS_COLOR, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
                    {data: this.data[6], color: "#FFEED7", bars: {show:true, lineWith: 1, barsWith: 1, full: true}},
                    {data: all_data.data[ind0], label: "Отношения ch: "+this.getChannel() },
                    {data: all_data.data[ind1], label: "КИХ Филтр ch"+this.getChannel()},
                    {data: all_data.data[ind2], label: "Корреляция ch"+this.getChannel()}
                ];
                break;
            case TYPE_DATA_F2P_SUMMS:
                data_ = [
                    {data: this.isEvents, color: EVENTS_COLOR, bars: {show:true, lineWith: 1, barsWith: 0.05, full: true}},
                    {data: this.data[6], color: "#FFEED7", bars: {show:true, lineWith: 1, barsWith: 1, full: true}},
                    {data: all_data.data[ind0], label: "Summ F2p 2 ch: "+this.getChannel() },
                    {data: all_data.data[ind1], label: "Sigma 2 ch: "+this.getChannel()}
                ];
                break;
            default:
                return;
        };
        var miny2 = all_data.min_max[ind1].min; var maxy2 = all_data.min_max[ind1].max;
        var miny1 = all_data.min_max[ind0].min; var maxy1 = all_data.min_max[ind0].max;
        if(this.getType() !== TYPE_DATA_DMA){ // добавим events
            if(this.getType() === TYPE_DATA_CRITERY){
                miny2 = -1.1; maxy2 = 1.1;
//            data_.unshift(this.data[3]);
//            miny2 = -1; maxy2 = 1;
//            miny1 = this.min_max.min < -1e-5 ? -1e-5 : this.min_max.min;
            } else if(this.getType() === TYPE_DATA_F2P_SUMMS){
                miny1 = miny1 < miny2 ? miny1 : miny2;
                maxy1 = maxy1 > maxy2 ? maxy1 : maxy2;
                miny1 -= (maxy1 - miny1) * 0.1; maxy1 += (maxy1 - miny1) * 0.1;
                miny2 -= (maxy2 - miny2) * 0.1; maxy2 += (maxy2 - miny2) * 0.1;
            } else if(this.getType() === TYPE_DATA_RATIOS || this.getType() === TYPE_DATA_DIFF){
                miny1 = miny1 < this.min3 ? miny1 : this.min3;  
                maxy1 = maxy1 > this.max3 ? maxy1 : this.max3;  
                miny1 -= (maxy1 - miny1) * 0.1;
                maxy1 += (maxy1 - miny1) * 0.1;
                miny2 = all_data.min_max[ind2].min; var maxy2 = all_data.min_max[ind2].max;

//                maxy2 = 1.1; miny2 = -1.1;
            
            } else if(this.getType() === TYPE_DATA_DIFF2){
                miny1 = miny1 < this.min3 ? miny1 : this.min3;  
                maxy1 = maxy1 > this.max3 ? maxy1 : this.max3;  
                miny1 -= (maxy1 - miny1) * 0.1;
                maxy1 += (maxy1 - miny1) * 0.1;
                miny2 = all_data.min_max[ind2].min; var maxy2 = all_data.min_max[ind2].max;
            }
        } else {
            miny1 = -5; maxy1 = 5;
        }
        $.plot(this.canvas,
        data_, {
//            xaxis:{
//                mode: "time",
//                timeformat: "%S"
//            },
            legend: {
                position: 'nw'
            },
            y2axis: {
                color: '#C0D800',
                min: miny2,
                max: maxy2
            },
            yaxis: {
                color: '#000080',
//                tickFormatter: function(val,axis){
//                    return val.toExponential(3);
//                },
                min: miny1,
                max: maxy1
            },
            series: {
                lines: {
                    show: true,
                    lineWidth :1
                }
            }
        }
        );
        var tse = new Date();
    };
    
    this.getPartsQuery = function(){
        var ind0 = this.index *3; var ind1 = this.index * 3 + 1; var ind2 = this.index * 3 + 2; var ret = "";
        switch(this.getType()){
//            case TYPE_DATA_DMA:
//               ret = "tp"+ind0+"="+TYPE_DATA_DMA+"&ch"+ind0+"="+this.getChannel()+"&r"+ind0+"=0";
//               ret += "tp"+ind1+"="+TYPE_NO_DATA;
//               break;
           case TYPE_DATA_RATIOS:
               var second = this.getRatio() === 0 ? 1 : 0;
               ret = "tp"+ind0+"="+TYPE_DATA_AVR_RATIOS+"&ch"+ind0+"="+this.getChannel()+"&r"+ind0+"="+this.getRatio();
               ret += "&tp"+ind1+"="+TYPE_DATA_3_SIGMA+"&ch"+ind1+"="+this.getChannel()+"&r"+ind1+"="+this.getRatio();
               ret += "&tp"+ind2+"="+TYPE_DATA_CORREL+"&ch"+ind2+"=0&r"+ind2+"=0";
               
               break;
           case TYPE_DATA_SUMMS:
               ret = "tp"+ind0+"="+TYPE_DATA_SUMMS+"&ch"+ind0+"="+this.getChannel()+"&r"+ind0+"="+this.getRatio();
               ret += "&tp"+ind1+"="+TYPE_NO_DATA+"&ch"+ind1+"=0&r"+ind1+"=0";
               ret += "&tp"+ind2+"="+TYPE_NO_DATA+"&ch"+ind2+"=0&r"+ind2+"=0";
               break;
            case TYPE_DATA_DIFF:
               ret = "tp"+ind0+"="+TYPE_DATA_AVR_RATIOS+"&ch"+ind0+"="+this.getChannel()+"&r"+ind0+"=0";
               ret += "&tp"+ind1+"="+TYPE_DATA_DIFF+"&ch"+ind1+"="+this.getChannel()+"&r"+ind1+"=0";
               ret += "&tp"+ind2+"="+TYPE_DATA_DIFF_ALL_CHANNELS+"&ch"+ind2+"=0&r"+ind2+"=0";
               break;
//            case TYPE_DATA_DIFF:
//               ret = "tp"+ind0+"="+TYPE_DATA_AVR_RATIOS+"&ch"+ind0+"="+this.getChannel()+"&r"+ind0+"=0";
//               ret += "&tp"+ind1+"="+TYPE_DATA_DIFF+"&ch"+ind1+"="+this.getChannel()+"&r"+ind1+"=0";
//               ret += "&tp"+ind2+"="+TYPE_DATA_CORREL+"&ch"+ind2+"=0&r"+ind2+"=0";
//               break;
           case TYPE_DATA_DIFF2:
               ret = "tp"+ind0+"="+TYPE_DATA_AVR_RATIOS+"&ch"+ind0+"="+this.getChannel()+"&r"+ind0+"=0";
               ret += "&tp"+ind1+"="+TYPE_DATA_CRITERY+"&ch"+ind1+"="+this.getChannel()+"&r"+ind1+"=0";
               ret += "&tp"+ind2+"="+TYPE_DATA_3_SIGMA+"&ch"+ind2+"="+this.getChannel()+"&r"+ind2+"=0";
               break;
           case TYPE_DATA_F2P_SUMMS:
               ret = "tp"+ind0+"="+TYPE_DATA_F2P_SUMMS+"&ch"+ind0+"="+this.getChannel()+"&r"+ind0+"="+this.getRatio();
               ret += "&tp"+ind1+"="+TYPE_DATA_3_SIGMA+"&ch"+ind1+"="+this.getChannel()+"&r"+ind1+"="+this.getRatio();
               ret += "&tp"+ind2+"="+TYPE_NO_DATA+"&ch"+ind2+"=0&r"+ind2+"=0";
               break;
       }
//        var ret = "tp"+this.index+"="+this.getType()+"&ch"+this.index+"="+this.getChannel()+"&r"+this.index+"="+this.getRatio();
        return ret;
    };
};
/*******************************************************/

var ts, te;
var ccc = 0;

/***************** full panel graphics and settings object ***********************/
function getGraphics(){
//    var queryStr = URL_+"?command=get_graphics4";
//    lp.ligth(ccc); ccc++;
//    if(iAmPaint) return;
    ts = new Date();
    var queryStr = URL_+"?command=get_all_states4";
    for(var i=0; i<4; i++){
        queryStr += "&"+grPanels[i].getPartsQuery();
    }
    if(!needChangeAllData){
        queryStr += "&time="+all_data.lastTime+"&ttime="+activeTime;
    }
    else {
        queryStr += "&time=0&ttime="+activeTime;
        needChangeAllData = false;
        
    }
        
//    $("#console").text(queryStr);
    $.ajax({
        type: "GET",
        url: queryStr,
        dataType: "xml",
        success: function(xml){
            var ts1, te1;
            ts1 = new Date();
            fullAllDataGraphics(xml);
            te1 = new Date();
            
            lp.ligth(all_data.evMask);
            
            $(xml).find("cdata").each(function(){
                var ch = parseInt($(this).attr("ch"));
                var kk = $(this).attr("kk");
                var pkk01 = $(this).attr("pkk01");
                var pkk02 = $(this).attr("pkk02");
                var pcorr01 = $(this).attr("pcorr01");
                for(var i=0; i<4; i++){
                    if(panelInFocus !== i){
                        if(grPanels[i].getChannel() === ch){
                           grPanels[i].setKK(kk);
                           grPanels[i].setPorog1(pkk01);
                           grPanels[i].setPorog2(pkk02);
                           grPanels[i].setCorrel(pcorr01);
                           grPanels[i].setXMLZones($(this));
                        }
                    };
                }
            });
            $(xml).find("reader").each(function(){
                var type = parseInt($(this).attr("type"));
                var name = $(this).attr("fname");
                var nnn = new Array();
                nnn = name.split("/");
                var softState = parseInt($(this).attr("sstate"));
                var hardState = parseInt($(this).attr("hstate"));
                var color = "darkBlue"; var t = "";
                if(hardState === 0){
                    t += '<color="darkBlue">Работает нормально </color>';
//                    $("#dev_state").text("Работает нормально");
                } 
                if((hardState & 1)===1){
                    t += '<color="Red">Ошибка привода... </color>';
//                    $("#dev_state").text("Ошибка привода...");
//                    color = "Red";
                } 
                if((hardState & 2)===2){
                    t += '<color="Red">Ошибка лазерных датчиков... </color>';
//                    $("#dev_state").text("Ошибка лазерных датчиков...");
//                    color = "Red";
                } 
                if((hardState & 4)===4){
                    t += '<color="darkGreen">Нажата кнопка ГРИБ... </color>';
//                    $("#dev_state").text("Нажата кнопка ГРИБ...");
//                    color = "darkRed";
                } 
                if((hardState & 8)===8){
                    t += '<color="darkGreen">Конвейер остановлен... </color>';
//                    $("#dev_state").text("Конвейер остановлен...");
//                    color = "darkGreen";
                } 
                if((hardState & 16)===16){
                    t += '<color="darkGreen">Катушка поднята... </color>';
//                    $("#dev_state").text("Катушка поднята...");
//                    color = "darkGreen";
                } 
                if((hardState & 32)===32){
                    t += '<color="darkGreen">Ручное управление... </color>';
//                    $("#dev_state").text("Катушка поднята...");
//                    color = "darkGreen";
                }
                if((hardState & 64)===64){
                    t += '<color="Red">Ошибка усилителя... </color>';
//                    $("#dev_state").text("Катушка поднята...");
//                    color = "darkGreen";
                }
                $("#dev_state").html(t);
//                if(hardState === 0){
//                    $("#dev_state").css("color", "darkBlue");
//                } else {
//                    $("#dev_state").css("color", "darkRed");
//                    $("#dev_state").text("Не работает");
//                }
                if(softState === 1){
                    $("#soft_state").text("Работает нормально");
                    $("#soft_state").css("color", "darkBlue");
                } else {
                    $("#soft_state").text("Не работает");
                    $("#soft_state").css("color", "darkRed");
                }
                var stype = "DMA"; var color = "darkBlue";
                if(type === 2) {
                    stype = "Dump файл: "+nnn[nnn.length-1];
                    color = "darkGreen";
                }
                if(type === 3) {
                    stype = "Фаил сумм: "+nnn[nnn.length-1];
                    color = "darkRed";
                }
                $("#spReader").text(stype);
                $("#spReader").css("color", color);
            });
            $(xml).find("dump").each(function(){
                var name = $(this).attr("fname");
                var state = parseInt($(this).attr("state"));
                if(state === 0) stateControlPanels.setSaveDumpState(false, name);
                else stateControlPanels.setSaveDumpState(true, name);
            });
            $(xml).find("non_stop").each(function(){
                var state = parseInt($(this).attr("state"));
                if(state === 1){
                    $("#cbNonStop").prop("checked", true);
                    $("#spNonStop").css("color", "darkRed");
                    $("#spNonStop").text("включен");
                } else {
                    $("#cbNonStop").prop("checked", false);
                    $("#spNonStop").css("color", "darkBlue");
                    $("#spNonStop").text("выключен");
                }
            });
            $(xml).find("sdump").each(function(){
                var name = $(this).attr("fname");
                var state = parseInt($(this).attr("state"));
                if(state === 0) stateControlPanels.setSaveSummState(false, name);
                else stateControlPanels.setSaveSummState(true, name);
            });
            $(xml).find("tunning").each(function(){
                var state = parseInt($(this).attr("state"));
                if(state === 0) {
                    stateControlPanels.setTunningState(false);
                    stateControlPanels.setDisabled(true);
                } else {
                   stateControlPanels.setTunningState(true);
                   stateControlPanels.setDisabled(false);
                }
            });
            $(xml).find("time").each(function(){
                if(activeTime === 1){
                    $("#iTime").prop("value", $(this).attr("dt"));
                } else {
                    $("#iTime").text($(this).attr("dt"));
                }
            });
            
            te1 = new Date();
//            var ttt = $("#console").text(); ttt += " ather - "+(te1.getTime() - ts1.getTime());
//            $("#console").text(ttt);
            for(var i=0; i<4; i++){
                grPanels[i].fullData(xml);
                grPanels[i].paint();
            }
            te1 = new Date();
            currentTTL = te1 - ts1;
//            $("#console").text($("#console").text() + " html "+ (te1.getTime() - ts.getTime()));
            var i = 0;
        },
        complete: function(XHR, textStatus){
            iAmPaint = false;
            te = new Date();
//            $("#console").text($("#console").text() + " всего "+ (te.getTime() - ts.getTime()));
            setTimeout(getGraphics, 1000);
        },
        errors: function(XHR, textStatus){
            iAmPaint = false;
            setTimeout(getGraphics, 1000);
        }
    });

//    queryStr = URL_+"?command=get_calibrate4";
//    $.ajax({
//        type: "GET",
//        url: queryStr,
//        dataType: "xml",
//        success: function(xml){
//        }
//    });
};

function grControls(){

    this.bClear = $("#graphics_clear");
    this.clearGraphics = function(){
       panelInFocus = -1;
       lastMyTime.setLastTime(0);
        
//       $.ajax({
//           url: URL+"?command=clear_graphics",
//           type: "GET",
//           dataType: "xml",
//           success: function(xml){
//
//           }
//       }); 
    };
    this.bClear.bind("click",this.clearGraphics);

    this.bStore = $("#recept_store");
    this.setReceptStore = function(){
        panelInFocus = -1;
        $.ajax({
            url: URL_+"?command=save_as_recept",
            type: "GET",
            dataType: "xml",
            success: function(xml){
                $(xml).find("status").each(function(){
                    if($(this).text().trim().toUpperCase() === "OK"){
                        alert("Calibrate data store to recept success...");
                    } else {
                        alert("Error store calibrate...");
                    }
                })
            }
        });        
    };
    
    this.lastTimeGraphic = "0";
    
    this.bStore.bind("click", this.setReceptStore);

    this.bToDefault = $("#recept_defautl");
    this.setDefaultRecept = function(e){
        panelInFocus = -1;
         $.ajax({
            url: URL_+"?command=set_default_recept",
            type: "GET",
            success: function(xml){
                $(xml).find("status").each(function(){
                    if($(this).text().trim().toUpperCase() === "OK"){
                        alert("Load default recept success...");
                    } else {
                        alert("Error load default recept...");
                    }
                });
                
            }
        });
       
    };
    this.bToDefault.bind("click", this.setDefaultRecept);
    
    this.cbTunning = $("#cbTunning"); // Режим настройки ККЗ

    this.setTunningState = function(on_off){
        var spTunning = $("#spTunning");
        var cbTunning = $(this.cbTunning);
        if(on_off){
            $(cbTunning).prop("checked", true);
            $(spTunning).text("включен");
            $(spTunning).css("color", "darkRed");
        } else {
            $(cbTunning).prop("checked", false);
            $(spTunning).text("выключен");
            $(spTunning).css("color", "darkBlue");
        }
    };

    this.bAuth = $("#submitbtn");
    this.bAuth.click(function(){
        var str = URL_+"?command=set_tunning&state=1&passwd="+$("#password").val();
        $.ajax({
            url: str,
            type: "GET",
            dataType: "xml",
            success: function(xml){
                $(xml).find("tunning").each(function(){
                    if($(this).attr("auth") === "1"){
                        stateControlPanels.setTunningState(true);
                        $(document).trigger("mouseup");
                    } else {
                        $("#msg").text("Неправильный пароль...");
                    }
                });
            }
        });
    });

    this.cbTunning.bind("change", function(e, on){
        if($("#cbTunning").prop("checked")){
            $("#frmsignin").toggle('fast',function() {
                            $("#password").val("");
                            $('#username').focus();				
                    });
            $(this).toggleClass("btnsigninon");	
            $('#msg').empty();
        } else {
//        if(on === true || on === false){
//            $(stateControlPanels.cbTunning).prop("checked", on);
//        }
            var on_off = $(stateControlPanels.cbTunning).prop("checked") ? "1" : "0";
            $.ajax({
                url: URL_+"?command=set_tunning&state="+on_off,
                type: "GET",
                dataType: "xml",
                success: function(xml){
                        if($(this).text().trim().toUpperCase() === "OK"){
                            alert("Set tunning success...");
                        } else {
                            alert("Set tunning failed...");
                        }
                }
            });
        }
    });
    
    this.cbNonStop = $("#cbNonStop");
    this.cbNonStop.bind("change", function(e){
        var on_off = $(stateControlPanels.cbNonStop).prop("checked") ? "0" : "1";
        var strQuery = URL_+"?command=set_non_stop&state="+on_off;
        $.ajax({
            url: strQuery,
            type: "GET",
            dataType: "xml",
            success: function(xml){
                $(xml).find("state").each(function(){
                    if($(this).text().trim().toUpperCase() === "OK"){
                        alert("Non stop regime change success...");
                    }
                });
            }
        });
        
    });
    
    this.cbSaveDump = $("#cbSaveDump");
    this.setSaveDumpState = function(on_off, name){
        
        $(stateControlPanels.cbSaveSumm).prop("checked", on_off);
        var spSaveDMA = $("#spSaveDMA");
        if(on_off){
            var nnn = name.split("/");
            $(spSaveDMA).text("включен "+nnn[nnn.length-1]);
            $(spSaveDMA).css("color", "darkRed");
        } else {
            $(spSaveDMA).text("выключен");
            $(spSaveDMA).css("color", "darkBlue");
        }
    };
    this.cbSaveDump.bind("change", function(e){
        var on_off = $(stateControlPanels.cbSaveDump).prop("checked") ? "1" : "0";
        var strQuery = URL_+"?command=set_save_file_state&state="+on_off
        $.ajax({
            url: strQuery, 
            type: "GET",
            dataType: "xml",
            success: function(xml){
                    if($(this).text().trim().toUpperCase() === "OK"){
                        alert("Set save fail success...");
                    } else {
                        alert("Set save fail failed...");
                    }
                
            }
        });
    });
    
    this.cbSaveSumm = $("#cbSaveSumm");
    this.setSaveSummState = function(on_off, name){
        var spSaveSumm = $("#spSaveSumm");
        $(stateControlPanels.cbSaveSumm).prop("checked", on_off);
        if(on_off){
            var nnn = name.split("/");
            $(spSaveSumm).text("включен "+nnn[nnn.length-1]);
            $(spSaveSumm).css("color", "darkRed");
        } else {
            $(spSaveSumm).text("выключен");
            $(spSaveSumm).css("color", "darkBlue");
        }
    };
    this.cbSaveSumm.bind("change", function(e){
        var on_off = $(stateControlPanels.cbSaveSumm).prop("checked") ? "1" : "0";
        var strQuery = URL_+"?command=set_summ_dump_file_state&state="+on_off;
        $.ajax({
            url: strQuery, 
            type: "GET",
            dataType: "xml",
            success: function(xml){
                    if($(xml).text().trim().toUpperCase() === "OK"){
                        alert("Set save summ fail success...");
                    } else {
                        alert("Set save summ fail failed...");
                    }
                
            }
        });
    });
    
    this.getStrTime = function(str){
        if(str === ""){
            if(activeTime === 1){
                str = $(this.iTime).val();
            } else {
                str = $(this.iTime).text();
                var t0 = str.split(" ");
                var tt0 = t0[0].split(".");
                str += tt0[2]+"-"+tt0[1]+"-"+tt0[0]+"T"+t0[1];
            }
            return str;
        } else {
            var now = new Date();
            str = now.getFullYear()+"-"+now.getMonth()+"-"+now.getDate()+"T"+now.getHours()+":"+now.getMinutes()+":"+now.getSeconds();
//            str = "?command=set_time&stime="+str;
            return str;
        }
    };
    
    this.bSetCompTime = $("#bSetCompTime");
    this.bSetCompTime.bind("click",function(){
        var str = URL_+"?command=set_time&stime=";
            str += stateControlPanels.getStrTime("1");
        var i = 0;
        $.ajax({
            url: str,
            type: "GET",
            dataType: "xml",
            success: function(xml){
                    if($(xml).text().trim().toUpperCase() === "OK"){
                        alert("Sytem time changed success...");
                    } else {
                        alert("Error changed system files...");
                    }
            }
        });
    });
    
    this.iTime = $("#iTime");
    this.iTime.bind("change",function(e){
       if(activeTime === 1){
           var stime=$(stateControlPanels).getStrTime(""); 
           var str = URL_+"?command=set_time&stime="+stime;
           $.ajax({
              url: URL_+"?command=set_time&stime="+stime,
              type: "GET",
              dataType: "xml",
              success: function(xml){
                    if($(this).text().trim().toUpperCase() === "OK"){
                        alert("Sytem time changed success...");
                    } else {
                        alert("Error changed system files...");
                    }
              },
              errors: function(){
                    alert("Error changed system files...");
              }
           });
       } 
    });
    
    this.listControls = new Array(this.iTime, this.bClear, this.bSetCompTime, this.bStore, this.bToDefault, this.cbSaveDump, this.cbSaveSumm, this.cbNonStop);
    for(var i = 0; i < 4; i++){
        this.listControls.push(grPanels[i].kk, grPanels[i].correl, grPanels[i].porog1, grPanels[i].porog2);
    }
    
    this.setDisabled = function(on_off){
        for(var i=0; i < this.listControls.length; i++){
            $(this.listControls[i]).prop("disabled", on_off);
        }
    }
    
    this.getAllStates = function(){
        var str = URL_+"?command=get_all_states4&ttime="+activeTime;
        $.ajax({
            url: str,
            type: "GET",
            dateType: "xml",
            success: function(xml){
//                $(xml).find("reader").each(function(){
//                    var type = parseInt($(this).attr("type"));
//                    var name = $(this).attr("fname");
//                    var nnn = new Array();
//                    nnn = name.split("/");
//                    var softState = parseInt($(this).attr("parser")) & parseInt($(this).attr("reader"));
//                    var hardState = parseInt($(this).attr("hstate"));
//                    if(hardState === 0){
//                        $("#dev_state").text("Работает нормально");
//                        $("#dev_state").css("color", "darkBlue");
//                    } else {
//                        $("#dev_state").text("Не работает");
//                        $("#dev_state").css("color", "darkRed");
//                    }
//                    if(softState === 1){
//                        $("#soft_state").text("Работает нормально");
//                        $("#soft_state").css("color", "darkBlue");
//                    } else {
//                        $("#soft_state").text("Не работает");
//                        $("#soft_state").css("color", "darkRed");
//                    }
//                    var stype = "DMA"; var color = "darkBlue";
//                    if(type === 2) {
//                        stype = "Dump файл: "+nnn[nnn.length-1];
//                        color = "darkGreen";
//                    }
//                    if(type === 3) {
//                        stype = "Фаил сумм: "+nnn[nnn.length-1];
//                        color = "darkRed";
//                    }
//                    $("#spReader").text(stype);
//                    $("#spReader").css("color", color);
//                });
//                $(xml).find("dump").each(function(){
//                    var name = $(this).attr("fname");
//                    var state = parseInt($(this).attr("state"));
//                    if(state === 0) stateControlPanels.setSaveDumpState(false, name);
//                    else stateControlPanels.setSaveDumpState(true, name);
//                });
//                $(xml).find("sdump").each(function(){
//                    var name = $(this).attr("fname");
//                    var state = parseInt($(this).attr("state"));
//                    if(state === 0) stateControlPanels.setSaveSummState(false, name);
//                    else stateControlPanels.setSaveSummState(true, name);
//                });
//                $(xml).find("tunning").each(function(){
//                    var state = parseInt($(this).attr("state"));
//                    if(state === 0) {
//                        stateControlPanels.setTunningState(false);
//                        stateControlPanels.setDisabled(true);
//                    } else {
//                       stateControlPanels.setTunningState(true);
//                       stateControlPanels.setDisabled(false);
//                    }
//                });
//                $(xml).find("time").each(function(){
//                    if(activeTime === 1){
//                        $("#iTime").prop("value", $(this).attr("dt"));
//                    } else {
//                        $("#iTime").text($(this).attr("dt"));
//                    }
//                });
            }
        });
    };
};


function createAuthForm(){	
//    $('#cbTunning').click(function(e) {	// открываем форму для ввода пароля	
//        e.preventDefault();         // перехватываем действие по умолчанию change
//        if(!$("#cbTunning").prop("checked")){
//            $("#frmsignin").toggle('fast',function() {	
//                            $('#username').focus();				
//                    });
//            $(this).toggleClass("btnsigninon");			
//            $('#msg').empty();
//        } else {
//            $("#cbTunning").trigger("change", true);
//        }
//    });

    $('#cbTunning').mouseup(function() {
            return false;
    });

    $(document).mouseup(function(e) {			 // скрываем форму при нажатии во вне ее
            if($(e.target).parents('#frmsignin').length===0) {			
                    $('.btnsignin').removeClass('btnsigninon');		
                    $('#frmsignin').hide('fast');
            };
    });
    
//    $("#submitbtn").click(function(){
//        var str = URL_+"?command=set_tunning&state=1&passwd="+$("#password").val();
//        var i = 0;
//        $.ajax({
//            url: str,
//            type: "GET",
//            dataType: "xml",
//            success: function(xml){
//                $(xml).find("auth").each(function(){
//                    if($(this).attr("auth") === "1"){
//                        $("#cbTunning").trigger("change", true);
//                        $(document).trigger("mouseup");
//                    } else {
//                        $("#msg").text("Неправильный пароль...");
//                    }
//                });
//            }
//        });
//    });
};

function getTimeOut(){
    for(var i=0; i < 4; i++){
        var t = grPanels[i].getType();
        if(t === 0){ // if DMA type
           return 1000; 
        }
    }
    return 500;
}

var myTimer;

$(document).ready(function(){
    
    
    grPanels[0] = new grObject(0, null);
    grPanels[1] = new grObject(1, null);
    grPanels[2] = new grObject(2, null);
    grPanels[3] = new grObject(3, null);

    stateControlPanels = new grControls();
//    stateControlPanels.getAllStates();
    lastMyTime = new LastMyTime("0");

imLamps_red = new Image();
imLamps_red.src = "images/lamp_ellow.png";

imLamps_white = new Image();
imLamps_white.src = "images/lamp_white_3.png";

    lp = new lampsPanel();
    
  document.onkeydown = function(e) {
    e = e || window.event;
    if (e.ctrlKey)
        if(e.keyCode === 0x20) {
            for(var i=0; i<4; i++){
                grPanels[i].storePicture("storePicture_"+i+".png");
            }


        }
    return true;
  }
    
    
    createAuthForm();
// использовать     
//    getGraphics();

//    $("#console").append("<span style=\"color: #0F0\">Timer started.</span<br />");
//    myTimer = $.timer(timeOutRefresh, function() {
        
//            $("#console").append("Timer completed.<br>");
        getGraphics();
//        var t = msec > currentTTL ? msec : currentTTL;
//        if((timeOutRefresh - t) > 100 || (timeOutRefresh - t) > 100){
//            timeOutRefresh = msec;
//            myTimer.stop();
//            myTimer.reset(timeOutRefresh);
//        }
});
//    $("#bSetCompTime").everyTime(1000, 'timer2', function(){
//            getGraphics();
//            timeOutRefresh = getTimeOut();
//    });
//        setInterval(function(){
    //        stateControlPanels.getAllStates();
//        }, timeOutRefresh);
//    alert("In ready to change is "+ob1.isReadyToChange());
//    
//    var rrr = document.getElementById("gr_coeff_0").focus();
//    
//    alert("In ready to change is "+ob1.isReadyToChange());
    
