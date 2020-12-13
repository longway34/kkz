// JavaScript Document

var imLamps = {
    red: new Image(),
    white: new Image()
};

imLamps.red.src = "images/lamp_ellow.jpg";
imLamps.white.src = "images/lamp_white_3.jpg";

lampsPanel = function(){
    this.lamps = new Array(6);
    for(var i=0; i < 6; i++){
            var name = "#l_"+i;
            this.lamps[i] = $(name)[0];
//            alert("create "+i+" as "+this.lamps[i]);
    }
    this.ligth = function(code){
        for(var i=0; i<6; i++){
            var mask = 1 << i;
            if((code & mask) === mask){
                this.lamps[i].src = imLamps.red.src; //getContext("2d").drawImage(imLamps.red);
            } else {
                this.lamps[i].src = imLamps.white.src; //getContext("2d").drawImage(imLamps.white);
            }
        }
    };
};

var count = 0;
var lp; 

$(document).ready(function(){
    
        lp = new lampsPanel();
        setInterval(function(){
        lp.ligth(count);
        count++; if(count === 63) count =0;
    }, 1000);
}); 