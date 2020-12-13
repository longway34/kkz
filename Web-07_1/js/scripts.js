/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

URL = document.baseURI;
if(URL.indexOf("localhos:8383") > 0){
   URL = "http://192.168.88.222:8000/index.html"; 
}
//URL = "http://192.168.2.160:8000/index.html";
//URL = "http://192.168.56.101:8000/index.html";

var elements;
var sqMin;
var parLen;
var parMin;

var channelsLists = new Array();
channelsLists[-1] = [0, 1, 2, 3, 4, 5];
channelsLists[-2] = [0, 1, 2];
channelsLists[-3] = [3, 4, 5];
for(var i=0; i<6; i++){
    channelsLists[i] = [i];
}

var addSigmaData;

var firstStep = true;

function Lamps_Panel(){
    function Lamps(index){
        this.lamp = $("#iLamp"+index);
        this.src_1 = "images/lamp_yellow.png";
        this.src_0 = "images/lamp_white.png";

        this.setLigth = function(light){
            if(light === 0){
                $(this.lamp).prop("src", this.src_0);
            } else {
                $(this.lamp).prop("src", this.src_1);
            }
        };
    }
    
    this.lamps = new Array(6);
    for(var ch=0; ch<6; ch++){
        this.lamps[ch] = new Lamps(ch);
    }
    
    this.setLight = function(code){
        for(var ch=0;ch<6;ch++){
            if((code & (1 << ch)) !== 0){
                this.lamps[ch].setLigth(1);
            } else {
                this.lamps[ch].setLigth(0);
            }
        }
    };
}

function addEvents(){
    function Len(index){
        this.len = $("#lLength"+index);
        this.set = function(data){
            $(this.len).text(data);
        };
    } 
    function Sigma(index){
        this.sigma = $("#lSigma"+index);
        this.set = function(data){
            $(this.sigma).text(data);
        };
    }
    this.lengths = new Array(6);
    this.sigmas = new Array(6);
    this.selfDiv = $("#divEventsDetail");
    this.selfTypeGr = $("#divAddGraphics")
    
    for(var index=0; index<6; index++){
        this.lengths[index] = new Len(index);
        this.sigmas[index] = new Sigma(index);
    }
    this.show;
    this.Show = function(on_off){
        this.show = on_off;
        if(this.show){
            $(this.selfDiv).css("display", "block");
            $(this.selfTypeGr).css("display", "block");
        } else {
            $(this.selfDiv).css("display", "none");
            $(this.selfTypeGr).css("display", "none");
        }
    };
    
    this.setParamas = function(xml){
        $(xml).find("last").each(function(){
            var ch = parseInt($(this).attr("ch"));
            var sig = parseFloat($(this).attr("lastSigma")).toFixed(1);
            var len = parseInt($(this).attr("lastLen"));
            if(len < 0 || len > 1000) len = 0;
            addSigmaData.lengths[ch].set(len);
            addSigmaData.sigmas[ch].set(sig);
        });
    };
    this.typeGraphic = $('input[name="cbTypeGraphic"]');
    this.typeGraphic.change(function(event){
        graphicsClear();
        var val = $(this).val();
    })
    this.getTypeGraphic = function(){
        var ret = -1;
        $(this.typeGraphic).each(function(){
            if($(this).prop("checked")){
                ret = $(this).val();
            }
        })
        return parseInt(ret);
    }
}

function controls(){
    
    this.calibrateDefault;
    this.calibrateCurrent;
//    this.formFocused = false;
    
    this.ttime = $("#tTime");
    this.bSetTime = $("#bSetTime");
/* Выбор номера колебровки 1 - 5 */    
    this.calibrate = $("#sCalibrate");
    this.getCalibrate = function(){
        var ret = $(vControls.calibrate).val();
        return parseInt(ret);
    };
//    this.divParabola = $("#divParabola");
//    
    this.chAddGraphics = $("#addGraphics");
    this.isAddGraphics = function(){
        var ret = $(vControls.chAddGraphics).prop("checked");
        return ret;
    }
    this.chAddGraphics.change(function(event){
        var ret = vControls.isAddGraphics();
        if(ret){
            addSigmaData.Show(true);
        } else {
            addSigmaData.Show(false);
        }
    });
 
    this.setSelectClibrate = function(num){
        var critery = "#sCalibrate [value='"+num+"']";
        $(vControls.calibrate).find("option").each(function(index, self){
            if(parseInt($(self).val()) === num){
                $(self).prop("selected", true);
            } else {
                $(self).prop("selected", false);
            }
        });
    };
    
    this.canvas = $("#divGrCanva");
    this.lSotfState = $("#lSotfState");
    this.lHardState = $("#lHardState");
    this.tNonGabarites = $("#lNG");
    this.tEventsCount = $("#lRZ");
/* Сброс счетчиков */
    this.bClearCount = $("#bCrearCounts");
    this.bClearCount.click(function(event){
        var url = URL+"?command=clear_counts";
        $.ajax({
            url: url,
            type: "GET",
            typeData: "xml",
            success: function(xml){
                var ret = $(this).find("state").first().attr("state");
            }
        });
    });

/* Выбор канала */    
    this.channel = $("#sChannel");
    this.getChannel = function(){
        var ret = $(vControls.channel).val();
        return parseInt(ret);
    };
    $(this.channel).change(function(event){
        graphicsClear();
        var val = $(vControls.channel).val(); 
//       alert("channels "+vControls.getChannelsCount()+" chanel "+val);
    });
    
/* Признак рабочей калибровки не активно даже при tunning mode меняется кнопкой*/
    this.isCurrent = $("#cbIsCurrentCalibrate");
    this.isCurrent.click(function(event){
        
//        alert("Change current calibrate to" + vControls.getCalibrate());
    });
/* Выбор количества коналов 1 2 6 */    
    this.channelsCount = $('input[name="channels"]');
    this.onChangeChannelsCount = function(num){
        $(vControls.channel).empty();
        switch(num){
            case 1:
                $(vControls.channel).append('<option value="-1">Все каналы</option>');
                break;
            case 2:
                $(vControls.channel).append('<option value="-1" disabled="disabled">Выберите часть</option>');
                $(vControls.channel).append('<option value="-2" selected="selected">Ближняя половина</option>');
                $(vControls.channel).append('<option value="-3">Дальняя половина</option>');
                break;
            case 6:
                $(vControls.channel).append('<option value="-1" disabled="disabled">Выберите канал</option>');
                $(vControls.channel).append('<option value="0" selected="selected">Канал 0</option>');
                $(vControls.channel).append('<option value="1">Канал 1</option>');
                $(vControls.channel).append('<option value="2">Канал 2</option>');
                $(vControls.channel).append('<option value="3">Канал 3</option>');
                $(vControls.channel).append('<option value="4">Канал 4</option>');
                $(vControls.channel).append('<option value="5">Канал 5</option>');
                break;
        }
        graphicsClear();
    };
    $(this.channelsCount).each(function(){
        $(this).change(function(){
            vControls.onChangeChannelsCount(parseInt($(this).val()));
        });
    });
    this.getChannelsCount = function(){
        var ret = -1;
        $(this.channelsCount).each(function(){
            var s = $(this).prop("checked");
            if(s){
                ret = parseInt($(this).val());
            }
        });
        return ret;
    };
    this.fsChannelsCount = $("#fsChannelsCount");
    this.fsChannelsCount.click(function(event){
//        alert(vControls.getCalibrate());
    });
    
   
// кнопка установки компьютерного времени доступна при Tunning mode     
    this.bSetTime.click(function(event){
        var ti = event.timeStamp;
        alert(ti);
    });
    
// поле установки значения сигма доступна при Tunning mode    
    this.sigma = $("#tSigma");
    this.getSigma = function(){
        var ret = $(this.sigma).val();
        return parseFloat(ret);
    };
    this.setSigma = function(_sigma, ather){
        $(this.sigma).val(_sigma);
        if(ather){
            $(vControls.sigma).addClass("ather");
        } else {
            $(vControls.sigma).removeClass("ather");
        }
    };
    
// поле установки значения длины доступна при Tunning mode    
    this.length = $("#tLength");
    this.getLength = function(){
        var ret = $(this.length).val();
        return parseInt(ret);
    };
    this.setLength = function(_len, ather){
        $(this.length).val(_len);
        if(ather){
            $(vControls.length).addClass("ather");
        } else {
            $(vControls.length).removeClass("ather");
        }
    };
// функции смены калибровочных значений    
    this.setCalibrate = function(){
// написать смену значения сигма и длины в выбранную калибровку в нужные каналы
        var channels = vControls.getChannel();
        var url = URL+"?command=set_calibrate_data&channel="+channels+"&calibrate="+vControls.getCalibrate()+"&sigma="+vControls.getSigma()+"&len="+vControls.getLength();
        $.ajax({
            url: url,
            type: "GET",
            typeData: "xml",
            success: function(xml){
                var ret = $(xml).find("state").first().attr("state");
            }
        });
    };
    this.formFocusedElements = $().add(this.length).add(this.sigma);
    this.formFocused;
    this.isFormFocused = function(){
        elements = null;
        elements = $("input:focus");
        return $(elements).length > 0;
    };
    $(this.length).focusin(function(event){
        vControls.formFocused = vControls.isFormFocused();
        var i = 0;
    });
    $(this.sigma).focusin(function(event){
        vControls.formFocused = vControls.isFormFocused();
        var i = 0;
    });
    $(this.length).focusout(function(event){
        vControls.formFocused = vControls.isFormFocused();
        var i = 0;
    });
    $(this.sigma).focusout(function(event){
        vControls.formFocused = vControls.isFormFocused();
        var i = 0;
    });
//    

    this.length.change(function(event){
        var ret = vControls.isFormFocused();
        vControls.setCalibrate();
    });
    this.sigma.change(function(event){
        var ret = vControls.isFormFocused();
        vControls.setCalibrate();
    });
    
    
// кнопка сохранения в рецепте текущих установок доступна при Tunning mode    
    this.bStoreSettings = $("#bStore");
    
// кнопка сброса изменений на значения по умолчанию доступна при Tunning mode 
    this.bRestoreSetting = $("#bRestore");
    this.bRestoreSetting.click(function(event){
// написать Восстановление из рецепта установок
//        alert("Восстановление из рецепта установок ");
        var url = URL+"?command=read_calibrate";
        $.ajax({
            url: url,
            type: "GET",
            typeData: "xml",
            success: function(xml){
                var ret = $(xml).find("state").first().attr("state");
            }
        });
    });

// кнопка установки выбранной калибровки как рабочей
    this.bSetAsCurrent = $("#bAsCurrent");
    this.bSetAsCurrent.click(function(event){
//        alert("Написать установку текущей калибровки как рабочую на "+vControls.getCalibrate());
        var url = URL+"?command=set_calibrate_num&num="+vControls.getCalibrate();
        $.ajax({
            url: url,
            type: "GET",
            typeData: "xml",
            success: function(xml){
               var ret = $(xml).find("state").first().attr("state");
            }
        });
    });
    
// флаг установки режима без остановки конвейера доступна при Tunning mode
    this.cbNonStopMode = $("#chNonStopMode");
    this.isNonStopMode = function(){
        return $(this.cbNonStopMode).prop("checked");
    };

    $(this.cbNonStopMode).change(function(event){
       var state = $(vControls.cbNonStopMode).prop("checked") ? "1" : "0";
       var url = URL+"?command=set_non_stop&state="+state;
       $.ajax({
           url: url,
           type: "GET",
           typeData: "xml",
           success: function(xml){
               var ret = $(xml).find("state").first().attr("state");
           }
       });
    });
/* Элементы для включения/выключения активности при калебровки*/    
    this.activateElementsGroup = $().add(this.bSetTime).
                                    add(this.bStoreSettings).
//                                    add(this.calibrate).
                                    add(this.bRestoreSetting).
                                    add(this.bStoreSettings).
                                    add(this.cbNonStopMode).
                                    add(this.bSetAsCurrent).
                                    add(this.sigma).
                                    add(this.length).
                                    add(this.chAddGraphics);
// форма ввода пароля    
    this.formPasswd = $("#dPasswd");
    this.bOk = $("#bPasswd");
    this.iClose = $("#modal_close");
    this.overlay = $("#overlay");
    this.tPasswd = $("#tPasswd");
    
    this.openPasswDialog = function(form){
        $(vControls.overlay).fadeIn(400, // снaчaлa плaвнo пoкaзывaем темную пoдлoжку
        function(){
//            $(vControls.formPasswd) 
                $(form)
                    .css('display', 'block') // убирaем у мoдaльнoгo oкнa display: none;
                    .animate({opacity: 1, bottom: '20px'}, 200); // плaвнo прибaвляем прoзрaчнoсть oднoвременнo сo съезжaнием вверх
        });
    };
    this.closePasswdDialog = function(form){
//        $(vControls.formPasswd)
        $(form)
                .animate({opacity: 0, bottom: '0px'}, 200,
                    function(){
                        $(this).css('display', 'none'); // делaем ему display: none;
                        $(vControls.overlay).fadeOut(400); // скрывaем пoдлoжку
//                        $(form).fadeOut(400); // скрывaем пoдлoжку
                });
    };

    this.bOk.click(function(event){
//        alert("Будет послана команда установки туннинг режима с паролем "+ $(vControls.tPasswd).val());
        var url = URL+"?command=set_tunning&passwd="+$(vControls.tPasswd).val()+"&state=1";
        $.ajax({
            url: url,
            type: "GET",
            typeData: "xml",
            success: function(xml){
                if($(xml).find("state").first().attr("state") === "OK"){
//                    alert("OK");
                    vControls.closePasswdDialog(vControls.formPasswd);
                } else {
                    alert("Пароль не правильный... "+$(xml).find("state").first().attr("state"));
                }
            }
//            errors: null
        });
    });
    
    this.iClose.click(function(){
        vControls.closePasswdDialog(vControls.formPasswd);
    });
    

// переменные и объекты формы подтверждения сохранения настроек
    this.formConfirmStore = $("#form_confirm_store");
//    this.vFCS_Calibrate = $("#ltColebrate");
//    this.vFCS_Channels = $("#ltChannels");
    this.vFCS_bOk = $("#bOkConfirm");
    this.vFCS_bCancel = $("#bCancelConfirm");
    this.vFCS_iCancel = $("#confirm_close");
    
    $(this.bStoreSettings).click(function(event){
        vControls.openPasswDialog(vControls.formConfirmStore);
    });
    
    $(this.vFCS_bOk).click(function(event){
        var url = URL+"?command=store_calibrate";
        $.ajax({
            url: url,
            type: "GET",
            typeData: "xml",
            success: function(xml){
                if($(xml).find("state").first().attr("state") === "OK"){
                    vControls.closePasswdDialog(vControls.formConfirmStore);
                } else {
                    alert("Что то пошло не так... "+$(xml).find("state").first().attr("state"));
                }
            }
        });
    });
    $(this.vFCS_bCancel).click(function(event){
        vControls.closePasswdDialog(vControls.formConfirmStore);
    });
    $(this.vFCS_iCancel).click(function(event){
        vControls.closePasswdDialog(vControls.formConfirmStore);
    });
    
// флаг установки режима Tunning mode
    this.cbTunningMode = $("#cbTunning");
    this.isTunningMode = function(){
        return $(this.cbTunningMode).prop("checked");
    };
    $(this.cbTunningMode).change(function(event){
        var tunning = $(vControls.cbTunningMode).prop("checked");
        if(tunning){
            $(vControls.cbTunningMode).prop("checked", false);
            vControls.openPasswDialog(vControls.formPasswd);
        } else {
            $(vControls.cbTunningMode).prop("checked", true);
            var url = URL+"?command=set_tunning&state=0";
            $.ajax({
                type: "GET",
                url: url,
                typeData: "xml",
                success: function(xml){
                    if($(xml).each("state").first().attr("state") !== "OK"){
                        alert("Error complite command");
                    }
                }
            });
        }
    });
    
    this.setTunningMode = function(mode){
        if(mode){
            vControls.activateElementsGroup.each(function(){
                $(this).prop("disabled", false);
            });
        } else {
            vControls.activateElementsGroup.each(function(){
                $(this).prop("disabled", true);
            });
        }
        $(vControls.cbTunningMode).prop("checked", mode);
    };
    
} 

var vControls;

function painStates(xml){
    var channels, calibrates;
// элемент states
    $(xml).find("states").each(function(){
// tunning & non_stop
        var mode = $(this).attr("tunning") === "1";
        if(mode !== vControls.isTunningMode()){
            vControls.setTunningMode(mode);
        }
        mode = $(this).attr("non_stop") === "1";
        $(vControls.cbNonStopMode).prop("checked", mode);
// calibrate current
        var num = parseInt($(this).attr("calibtare"));
        if(firstStep){
           vControls.setSelectClibrate(num);
           firstStep = false;
        }
        var curr = num === vControls.getCalibrate();
        $(vControls.isCurrent).prop("checked", curr);
// hstate sstate
        var sstate = parseInt($(this).attr("sstate"));
        var color = "darkBlue";
        var t = "ПО: ";
        if(sstate & 0xFF00 !== 0){
            t += "Не работает!!!";
            col = "darkRed";
        } else if(sstate & 1 !== 1){
            t += "В ожидании колебровки";
        } else {
            t += "Работает нормально";
        }
        $(vControls.lSotfState).text(t).css("color", color);
        
        color = "darkBlue";
        t = "ЗУ: ";
        var hardState = parseInt($(this).attr("hstate"));
        if(hardState === 0){
            t += 'Работает нормально';
        } else{
            color = "darkRed";
            if((hardState & 1)===1){
                t += 'Ошибка привода... ';
            } 
            if((hardState & 2)===2){
                t += 'Ошибка лазерных датчиков... ';
            } 
            if((hardState & 4)===4){
                t += 'Нажата кнопка ГРИБ... ';
            } 
            if((hardState & 8)===8){
                t += 'Конвейер остановлен... ';
            } 
            if((hardState & 16)===16){
                t += 'Катушка поднята... ';
            } 
            if((hardState & 32)===32){
                t += 'Регламент... ';
            }
            if((hardState & 64)===64){
                t += 'Ошибка усилителя... ';
            }
        }
        $(vControls.lHardState).text(t).css("color", color);
        
// non_gabarites events time
        var ng = parseInt($(this).attr("ng"));
        $(vControls.tNonGabarites).text(ng);
        var events = parseInt($(this).attr("events"));
        $(vControls.tEventsCount).text(events);
        var time_ = $(this).attr("dt");
        $(vControls.ttime).val(time_);
    });
    if(vControls.isAddGraphics()){
        $(xml).find("calibrates").each(function(){
            sqMin = parseFloat($(this).attr("sq"));
            parLen = parseInt($(this).attr("parLen"));
            parMin = parseInt($(this).attr("parMin"));
            
            $("#parLen").text(parLen);
            $("#parMin").text(parMin);
            $("#sqMin").text(sqMin);
        })
    }
// sigma Length
    if(!vControls.isFormFocused()){
        var minLen = 1e6, lastLen=0; var minSigma = 1e6, lastSigma=0;
        var findAtherSigma = false; var findAtherLength = false; var countAther = 0;
        $(xml).find("calibrate").each(function(){
            var cal = parseInt($(this).attr("num"));
            if(vControls.getCalibrate() === cal){
                var arrChannels = channelsLists[vControls.getChannel()];
                $(this).find("channel").each(function(){
                    var nch = parseInt($(this).attr("num"));
                    if($.inArray(nch, arrChannels) > -1){
                        var sig = parseFloat($(this).attr("sigma"));
                        var len = parseInt($(this).attr("len"));
                        if(countAther === 0){
                            minSigma = sig; lastSigma = sig;
                            minLen = len; lastLen = len;
                        } else {
                            if(sig !== minSigma){ 
                                if(sig < minSigma) minSigma = sig; 
                                findAtherSigma = true;
                            }
                            if(len !== minLen){ 
                                if(len < minLen) minLen = len; 
                                findAtherLength = true; 
                            }
                        }
                        countAther++;
                    }
                });
            }
        });
        vControls.setSigma(minSigma, findAtherSigma);
        vControls.setLength(minLen, findAtherLength);
    }    
    
};

function compliteData(xml){
    painStates(xml);
    fullData(xml, vControls.getChannel(), vControls.getSigma());
    paint(vControls.canvas, channelsLists[vControls.getChannel()]);
    addSigmaData.setParamas(xml);
}

var main_count = 0;

function perpetual_motion_mashine (){
    var typeGr = addSigmaData.getTypeGraphic();
    var data = "";
    switch(typeGr){
        case 0:
            data = "command=get_all_states4&time="+currentLastTime; break;
        case 1:
            data = "command=get_all_states5&time="+currentLastTime; break;
        case 2:
            data = "command=get_all_states6&time="+currentLastTime; break;
        default :
            data = "command=get_all_states4&time="+currentLastTime; break;
    }
    
    lamps_panel.setLight(event_mask_current);
    
    $.ajax({
//        type: "GET",
//        typeData: "xml",
//        url: url,
        data: data,
        success: function(xml){
            compliteData(xml);
        },
        errors: function(xml){
            alert(xml);
        },
        complete: function(xml){
            setTimeout(perpetual_motion_mashine, 1000);
        }       
    });
    main_count++;
    if(main_count > 63) main_count = 0;
    
}

var iLogo;
var lamps_panel;

$(document).ready(function(){
   vControls = new controls(); 
   
    $.ajaxSetup({
        url: URL,
        dataType: "xml",
        type: "GET",
        cache: false
    });
    iLogo = $("#iLogo");
    lamps_panel = new Lamps_Panel();

    addSigmaData = new addEvents();

    $(iLogo).click(function(event){
        perpetual_motion_mashine();
    });
    
    
});