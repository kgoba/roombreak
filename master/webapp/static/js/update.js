var minutes = 60;
var seconds = 0;
var nodeList = ["BOMB", "VALVE", "FLOOR", "RFID", "KEY", "PBX_Task1", "PBX_Task2", "P2K", "MAP", "WC", "SNAKE"];
var gameActive = false;

function getTimeString(minutes, seconds)
{
    return ("00" + minutes).slice(-2) + ":" + ("00" + seconds).slice(-2)
}

function setTime() {
    var min = $('#setminutes').val();
    //var sec = $('#setseconds').val();
    var sec = 0;
    $.getJSON('/_time', { "minutes": min, "seconds": sec });
}

function request(method, params, onSuccess) {
    var result = {};
    $.getJSON(method, params, function(response) {
        if (undefined != response.success && response.success) {
            hideError();
            onSuccess(response);
        }
        else {
            showError(response.error + ' (' + method + ')');
        }
    } )
    .fail(function() {
        showError("Nav atbildes" + ' (' + method + ')');        
    });
    return result;
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
    if (gameActive) {
        if (seconds == 0) {
            if (minutes > 0) {
                minutes -= 1;
            }
            seconds = 59;
        }
        else {
            seconds -= 1;
        }
    }
    $("#timeLeft").text(getTimeString(minutes, seconds));
}
    
function syncTime() {
    var response = request('/_time', {}, function(response) {
        if (undefined != response.minutes) {
            minutes = response.minutes;
        }
        if (undefined != response.seconds) {
            seconds = response.seconds;
        }        
    });
    return false;
}

function refreshStatus() {
    var response = request('/_status', {}, function(response) {
        if (undefined != response.status) {
            //$("#status").text(response.status);        
            gameActive = response.status == "active";

            var showPause = (response.status == "pause");
            var showPlay = (response.status == "active");
            var showService = (response.status == "service");

            if (showPause) {
                $("#statusPause").show();
                $("#status").text("PAUZE");
                //$(".btnPause").addClass("active");
                $(".btnStart").addClass("disabled");
            }
            else $("#statusPause").hide();

            if (showPlay) {
                $("#statusPlay").show();
                $("#status").text("AKTĪVA");
                //$(".btnPause").removeClass("active");
                $(".btnStart").addClass("active");
                $(".btnStart").addClass("disabled");
                //$(".btnPause").addClass("disabled");
            }
            else $("#statusPlay").hide();

            if (showService) {
                $("#statusService").show();
                $("#status").text("APKOPE");
                //$(".btnPause").removeClass("disabled");
                //$(".btnPause").removeClass("active");
                $(".btnStart").removeClass("active");
                if (undefined != response.gameEnabled && !response.gameEnabled) {
                    $(".btnStart").removeClass("disabled");
                }
                else {
                    $(".btnStart").addClass("disabled");
                }
            }
            else $("#statusService").hide();
        }

        if (undefined != response.gameEnabled) {
            if (response.gameEnabled) {
                $("#enableText").text("aktīvs");                    
                $("#enableOn").show();
                $("#enablePause").hide();
            }
            else {
                $("#enableText").text("pauze");                    
                $("#enableOn").hide();
                $("#enablePause").show();
            }
        }

        if (undefined != response.doorsOpen) {
            if (response.doorsOpen) {
                $("#doorState").text("ATVĒRTAS");                    
                $("#doorsOpen").show();
                $("#doorsClosed").hide();
            }
            else {
                $("#doorState").text("AIZVĒRTAS");                                        
                $("#doorsOpen").hide();
                $("#doorsClosed").show();
            }
        }
        
        for (var index = 0; index < nodeList.length; index++) {
            var name = nodeList[index];
            updateNode(name, response[name]);   
        }
    
        if (response.VALVE != undefined && response.VALVE.digit != undefined) {
            $("#VALVE .digit").text(response.VALVE.digit);        
        }        
    });
    
    return false;
}

function showError(message) {
    $('#errorAlert').removeClass("hidden");
    $('#errorMessage').text(message);    
}

function hideError() {
    $('#errorAlert').addClass("hidden");
}

function resetNode(name) {
    request('/_setDone', { id: name, done: false } );
}

function finishNode(name) {
    request('/_setDone', { id: name, done: true } );
}

function enterMaintenance() {
    request('/_setGameState', { state: 'service' } );
}

function startGame() {
    request('/_setGameState', { state: 'active' } );
}

function pauseGame() {
    request('/_setGameState', { state: 'pause' } );
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
        })('#' + name + ' .btnReset', '#' + name + ' .btnFinish', name);
    }
    
    $('.btnStart').bind('click', pauseGame);
    //$('.btnPause').bind('click', pauseGame);
    $('.btnService').bind('click', enterMaintenance);
    $('#btnSetTime').bind('click', setTime);

    syncTime();
    refreshStatus();
    
    setInterval( refreshStatus, 1000 );
    setInterval( syncTime, 15000 );
    setInterval( tickSecond, 1000 );
});
