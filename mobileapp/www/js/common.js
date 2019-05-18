app.initialize();

(function(){
    var ConvertBase = function (num) {
        return {
            from : function (baseFrom) {
                return {
                    to : function (baseTo) {
                        return parseInt(num, baseFrom).toString(baseTo);
                    }
                };
            }
        };
    };
    ConvertBase.bin2dec = function (num) {
        return ConvertBase(num).from(2).to(10);
    };
    ConvertBase.bin2hex = function (num) {
        return ConvertBase(num).from(2).to(16);
    };
    ConvertBase.dec2bin = function (num) {
        return ConvertBase(num).from(10).to(2);
    };
    ConvertBase.dec2hex = function (num) {
        return ConvertBase(num).from(10).to(16);
    };
    ConvertBase.hex2bin = function (num) {
        return ConvertBase(num).from(16).to(2);
    };
    ConvertBase.hex2dec = function (num) {
        return ConvertBase(num).from(16).to(10);
    };
    this.ConvertBase = ConvertBase;
})(this);

function createCookie(name,value,days) {
    if (days) {
        var date = new Date();
        date.setTime(date.getTime() + (days * 24 * 60 * 60 *1000));
        var expires = "; expires=" + date.toGMTString();
    }
    else {
        var expires = "";
    }
    document.cookie = name + "=" + value + expires + "; path=/";
}

function readCookie(name) {
    var nameEQ = name + "=";
    var ca = document.cookie.split(';');
    for(var i=0;i < ca.length;i++) {
        var c = ca[i];
        while (c.charAt(0)==' ') {
            c = c.substring(1,c.length);
        }
        if (c.indexOf(nameEQ) == 0) {
            return c.substring(nameEQ.length,c.length);
        }
    }
    return null;
}

function eraseCookie(name) {
    createCookie(name,"",-1);
}

function changelang(code){
        scope.$apply(function () {
            if(code=="RO"){
                scope.selectlang=0;
                createCookie("lang",0,365);
            } else {
                scope.selectlang=1;
                createCookie("lang",1,365);
            }
        });
}

var app = angular.module("afcApp", []);

app.controller('main', function ($scope, $http, $window) {
    $scope.string = {
    	"auth":["Autentificare","Login"],
        "email":["Adresă de email","Email address"],
        "pass":["Parola","Parolă"],
        "passconfirm":["Confirmați parola","Confirm your password"],
        "signin":["Accesați-vă contul","Access your account"],
        "create":["Creați un cont","Create a new account"],
        "cancel":["Renunță","Cancel"],
        "next":["Următorul pas","Next step"],
        "name":["Nume","Name"],
        "surname":["Prenume","Surname"],
        "ph_name":["Ion","John"],
        "ph_surn":["Popescu","Doe"],
        "ph_em":["ion.popescu@yahoo.ro","john.doe@gmail.com"],
        "ree":["Din nou","Again"],
        "scan":["Scanați cardul de călătorie","Scan your travelcard"],
        "done":["Finalizați","Finish"]
    };
});

function nextStep(){
    $('.modal').modal('hide');
    nfc.addTagDiscoveredListener(function(nfcEvent){
        navigator.vibrate(300);
        var tag = nfcEvent.tag;
        var tagId = nfc.bytesToHexString(tag.id);
        var readvalue = ConvertBase.hex2dec(tagId);
        alert(readvalue);
    }, function(){}, function(){
        alert('Error!')
    });
    $('#scanCardModal').modal('show');
}

$(document).ready(function(){
	scope = angular.element($("html")).scope();

	if(readCookie("lang")==null){
    	scope.$apply(function () { scope.selectlang=0; })
    	createCookie("lang",0,365);
    }else{
        scope.$apply(function () { scope.selectlang=parseInt(readCookie("lang")); });
    }
});

$('div.modal').on('show.bs.modal', function() {
    var modal = this;
    var hash = modal.id;
    window.location.hash = hash;
    window.onhashchange = function() {
        if (location.hash!=modal.id){
            $(modal).modal('hide');
        }
    }
});

$('div.modal').on('hidden.bs.modal', function() {
    var hash = this.id;
    history.replaceState('', document.title, window.location.pathname);
});

$('div.modal button.close').on('click', function(){
    window.history.back();
})

$('div.modal').keyup(function(e) {
    if (e.keyCode == 27){
        window.history.back();          
    }
});