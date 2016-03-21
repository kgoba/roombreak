var minutes = 60;
var seconds = 0;
var nodeList = ["BOMB", "VALVE", "FLOOR", "RFID", "KEY", "PBX", "P2K", "MAP", "WC", "SNAKE"];

function getTimeString(minutes, seconds)
{
    return ("00" + minutes).slice(-2) + ":" + ("00" + seconds).slice(-2)
}

function updateNode(name, data) {
    var idStatus = '#' + name
    var idOK = "#" + name + ' .iconOK'
    var idAlert = "#" + name + ' .iconAlert'
    $(idStatus).removeClass();
    if ((undefined != data) && data.alive) {
        if (data.done) {
            $(idStatus).addClass("success");
            $(idOK).show();
        }
        else {
            $(idOK).hide();
        }
        $(idAlert).hide();
    }
    else {
        $(idOK).hide();
        $(idStatus).addClass("danger");
        $(idAlert).show();
    }
}

function tickSecond() {
    if (seconds == 0) {
        if (minutes > 0) {
            minutes -= 1;
        }
        seconds = 59;
    }
    else {
        seconds -= 1;
    }
    $("#timeLeft").text(getTimeString(minutes, seconds));
}
    
function syncTime() {
    $.getJSON('/_time', {}, function(data) {
        if (undefined != data.minutes) {
            minutes = data.minutes;
        }
        if (undefined != data.seconds) {
            seconds = data.seconds;
        }
    });
    return false;
}

function refreshStatus() {
  $.getJSON('/_status', {}, function(data) {
    $("#status").text(data.status);
    
    if (data.doorsOpen) {
        $("#doorState").text("ATVĒRTAS");                    
        $("#doorsOpen").show();
        $("#doorsClosed").hide();
    }
    else {
        $("#doorState").text("AIZVĒRTAS");                                        
        $("#doorsOpen").hide();
        $("#doorsClosed").show();
    }
    
    for (var index = 0; index < nodeList.length; index++) {
        var name = nodeList[index];
        updateNode(name, data[name]);   
    }
    
    $("#VALVE .digit").text(data.VALVE.digit);
    
  });
  return false;
}

function resetNode(name) {
    $.getJSON('/_setDone', { id: name, done: false } );
}

function finishNode(name) {
    $.getJSON('/_setDone', { id: name, done: true } );
}

//$(function() {
$(document).ready(function(){
    $('a#refresh').bind('click', refreshStatus);
    
    for (var index = 0; index < nodeList.length; index++) {
        var name = nodeList[index];
        (function(id1, id2, name) {
            $(id1).bind('click', function() {
                resetNode(name);
            });
            $(id2).bind('click', function() {
                finishNode(name);
            });            
        })('#' + name + ' .btnReset', '#' + name + ' .btnFinish', name)
    }
        
    syncTime();
    refreshStatus();
    
    setInterval( refreshStatus, 2000 );
    setInterval( syncTime, 15000 );
    setInterval( tickSecond, 1000 );
});