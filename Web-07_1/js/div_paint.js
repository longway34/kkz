/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


var GR_POSITION_EVENT           =   12; // маска по каналам для JS
var GR_POSITION_HREADY          =   13;
var GR_POSITION_SREADY          =   14;
var GR_POSITION_TIME            =   15; // for JS

var S_POSITION_TIME          =   16; // from server
var S_POSITION_MARK          =   17; // from server
var S_POSITION_EV_MASK       =   18; // from server

var grMaxGraphicCount = 16; // 0 - 11 graphics for JS
                            // 12 - evMask
                            // 13 - hard ready;
                            // 14 - soft ready;
                            // 15 - time;
var grMaxLength = 1000;

var all_data = {
    len: 0,
    types: new Array(8),
    data: new Array(grMaxGraphicCount),
    min_max: new Array(grMaxGraphicCount),
    events: new Array(4),
    evMask: 0,
    lastTime: 0
};

for(var i=0; i<grMaxGraphicCount; i++){
    all_data.data[i] = new Array();
    all_data.min_max[i] = {min: 1e25, max: -1e21};
}

function fullAllDataGraphics(xml){
    
    $(xml).find("data").each(function(){
        var len = parseInt($(this).attr("len"));

        $(this).find("CTR").each(function(){
            var num = parseInt($(this).attr("num"));
            all_data.types[num] = parseInt($(this).attr("type"));
        });
        all_data.evMask = 0;
//        var _len = all_data.data[num].length;
        var begin = (all_data.len + len) - grMaxLength; 
        all_data.len += len; if(all_data.len > grMaxLength) all_data.len = grMaxLength;
        if(begin < 0) begin = 0;
        
        var newLength = new Array(grMaxGraphicCount);

        for(var i=0; i<grMaxGraphicCount; i++){
            all_data.min_max[i].max = -1e25;  all_data.min_max[i].min = 1e25;
            newLength[i] = all_data.data[i].length + len; if(newLength[i] > grMaxLength) newLength[i] = grMaxLength;
// Обрезали данные до нужной длины
            all_data.data[i] = all_data.data[i].slice(begin);
            newLength[i] = all_data.data[i].length;
// Перенумеровали и нашли новые минимумы - максимумы                                       
            for(var j=0; j < all_data.data[i].length; j++){
                if($.isArray(all_data.data[i][j])){
                    all_data.data[i][j] = [j, all_data.data[i][j][1]];
                } else {
                    all_data.data[i][j] = null;
                }
                if(i < 12) {
                    if(all_data.data[i][j][1] < all_data.min_max[i].min) all_data.min_max[i].min = all_data.data[i][j][1];
                    if(all_data.data[i][j][1] > all_data.min_max[i].max) all_data.min_max[i].max = all_data.data[i][j][1];
                }
// Определили последнее время                        
                if(i === GR_POSITION_TIME){
                    if(all_data.data[i][j][1] > all_data.lastTime) all_data.lastTime = all_data.data[i][j][1];
                }
                if(i === GR_POSITION_EVENT){
                    if(all_data.data[i][j] !== null){
                        all_data.evMask |= all_data.data[i][j][1];
                    }
                }
            }
        }
        
        for(var gr = 0; gr<grMaxGraphicCount; gr++){
            newLength[gr] = all_data.data[gr].length;
            $(this).find("d").each(function(){
                var x = parseInt($(this).attr("x"));
                var grX = newLength[gr] + x;
                var values = $(this).text().split(" ");
                var time = parseInt(values[S_POSITION_TIME]);
                var hWait = parseInt(values[S_POSITION_MARK]) & 1;
                var eWait = parseInt(values[S_POSITION_MARK]) & 2;
                var events = parseInt(values[S_POSITION_EV_MASK]);

                if(gr < 12){
                    all_data.data[gr][grX] = [grX, parseFloat(values[gr])];
                    if(all_data.data[gr][grX][1] < all_data.min_max[gr].min) all_data.min_max[gr].min = all_data.data[gr][grX][1];
                    if(all_data.data[gr][grX][1] > all_data.min_max[gr].max) all_data.min_max[gr].max = all_data.data[gr][grX][1];
                } else {
                    switch(gr){
                        case GR_POSITION_TIME:
// Определили последнее время                        
                            all_data.data[gr][grX] = [grX, time];
                            if(all_data.data[gr][grX][1] > all_data.lastTime) all_data.lastTime = all_data.data[gr][grX][1];
                            break;
                        case GR_POSITION_HREADY:
                            all_data.data[gr][grX] = [grX, hWait]; break;
                        case GR_POSITION_SREADY:
                            all_data.data[gr][grX] = [grX, eWait]; break;
                        case GR_POSITION_EVENT:
                            if(events !== 0){
                                all_data.data[gr][grX] = [grX, events]; break;
                                all_data.evMask |= events;
                            } else {
                                all_data.data[gr][grX] = null; break;
                            }
                    }
                }
            });
        }
    });
}