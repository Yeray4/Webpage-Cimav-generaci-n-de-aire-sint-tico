//Al cargar la pagina se envia un valor valor de control al esp32 para saber en que pagina se encuentra y tener una variable de control

document.addEventListener("DOMContentLoaded", function () {
  var valueToSend = "1";
  var url = "/ubi?ubi=" + encodeURIComponent(valueToSend);

  var xhttp = new XMLHttpRequest();

  xhttp.onreadystatechange = function () {
    if (xhttp.readyState == 4 && xhttp.status == 200) {
      console.log("Valor enviado al servidor con éxito");
    }
  };

  xhttp.open("GET", url, true);
  xhttp.send();


  setTimeout(ajaxCall, 500);//ejecuta un segund despues para alacanzar la varialbe de controlo dentro del esp32
});

//ajax para subir los datos de temperatura y humedad emuladas por sistema de control
var temp_control = new XMLHttpRequest();
var hum_control = new XMLHttpRequest();
function ajaxVariablesEmuladasControl() {   // se ejecuta cada hora y sube la temperatura y humedad pronosticada


  temp_control.onreadystatechange = function () {
    if (temp_control.readyState == XMLHttpRequest.DONE) {
      if (temp_control.status == 200) {
        updateDataEmular(temp_control.responseText);
      }
      else {
        console.log('error', temp_control);
      }
    }
  };

  hum_control.onreadystatechange = function () {
    if (hum_control.readyState == XMLHttpRequest.DONE) {
      if (hum_control.status == 200) {
        updateDataEmular2(hum_control.responseText);
      }
      else {
        console.log('error2', hum_control);
      }
    }
  };

  temp_control.open("GET", "GetTemp_Emular", true);
  temp_control.send();
  hum_control.open("GET", "GetHum_Emular", true);
  hum_control.send();
}
var tempEmulada = document.getElementById("vfinalt1");
var humEmulada = document.getElementById("vfinalh1");

function updateDataEmular(valor) {
  tempEmulada.innerHTML = valor;
}

function updateDataEmular2(valor) {
  humEmulada.innerHTML = valor;
}


//AL presionarse el boton se envia una peticion al servidor para comenzar la emulacion, cambia a color rojo, cambia el texo 
// y luego vuelve a cambiar  de texto despues de unos segundo. Al presionarlo por segunda vez cambia color y texto y manda 
// otra peticion al servidor para detener la emulacion
const boton = document.querySelector(".btnop");
const home = document.getElementById('home');
const extra = document.getElementById('extra');
home
var id = 1;
function GetEmularDetener() {
  // envia la peticion al servidor para comenzar y detener la emulacion
  // cambia el color y el texo al presionarse el boton

  boton.classList.toggle("cambiocolor2");

  if (id == 1) {
    console.log("btn estado: en ON");
    boton.innerHTML = "Emulando..";
    id = 0;
    GetEmular();
    setTimeout(printDetener, 3000);
    home.style.opacity = '0';
    home.disabled = true;
    extra.style.opacity = '0';
    extra.disabled = true;
    setTimeout(ajaxVariablesEmuladasControl, 500);
  } else {
    console.log("btn estado: en off");
    boton.innerHTML = "Comenzar ";
    id = 1;
    GetDetener();
    home.style.opacity = '1';
    home.disabled = false;
    extra.style.opacity = '1';
    extra.disabled = false;
  }

}

var tempPronosticada = document.getElementById("valort1");
var humPronosticada = document.getElementById("valorRH1");

function updateData(valor) {
  tempPronosticada.innerHTML = valor;
}

function updateData2(valor) {
  humPronosticada.innerHTML = valor;
}





var xmlhttp_temp_pro = new XMLHttpRequest();
var xmlhttp2_hum_pro = new XMLHttpRequest();
function ajaxCall() {   // se ejecuta cada hora y sube la temperatura y humedad pronosticada


  xmlhttp_temp_pro.onreadystatechange = function () {
    if (xmlhttp_temp_pro.readyState == XMLHttpRequest.DONE) {
      if (xmlhttp_temp_pro.status == 200) {
        updateData(xmlhttp_temp_pro.responseText);
      }
      else {
        console.log('error', xmlhttp_temp_pro);
      }
    }
  };

  xmlhttp2_hum_pro.onreadystatechange = function () {
    if (xmlhttp2_hum_pro.readyState == XMLHttpRequest.DONE) {
      if (xmlhttp2_hum_pro.status == 200) {
        updateData2(xmlhttp2_hum_pro.responseText);
      }
      else {
        console.log('error2', xmlhttp2_hum_pro);
      }
    }
  };

  xmlhttp_temp_pro.open("GET", "GetTemp_Pronos", true);
  xmlhttp_temp_pro.send();
  xmlhttp2_hum_pro.open("GET", "GetHum_Pronos", true);
  xmlhttp2_hum_pro.send();
}



// se ejecuta cada hora y sube la temperatura y humedad pronosticada
(function scheduleAjax() {
  var currentTime = new Date();
  console.log('hora', currentTime.getHours());

  // Calcular el tiempo hasta la próxima hora
  var nextHour = new Date(currentTime.getFullYear(), currentTime.getMonth(), currentTime.getDate(), currentTime.getHours() + 1, 0, 0);
  var timeToNextHour = nextHour.getTime() - currentTime.getTime();

  setTimeout(function () {
    ajaxCall();
    scheduleAjax();
  }, timeToNextHour);
})();

//envia una peticion al servidor para detener la emulacion
//dicho de otra manera envia un valor a la esp32 para enviar ese 
//dato al otro esp32 por uart y detener la ejecucion

function GetEmular() { // envia la peticion al servidor
  var valueToSend = "1";
  var url = "/Emular?estado=" + encodeURIComponent(valueToSend);

  var xhttp = new XMLHttpRequest();

  xhttp.onreadystatechange = function () {
    if (xhttp.readyState == 4 && xhttp.status == 200) {
      console.log("Valor enviado al servidor con éxito");
    }
  };

  xhttp.open("GET", url, true);
  xhttp.send();
}

//envia una peticion al servidor para detener la emulacion
//dicho de otra manera envia un valor a la esp32 para enviar ese 
//dato al otro esp32 por uart y detener la ejecucion
function GetDetener() { // envia la peticion al servidor
  var valueToSend = "0";
  var url = "/StopEmular?estado=" + encodeURIComponent(valueToSend);

  var xhttp = new XMLHttpRequest();

  xhttp.onreadystatechange = function () {
    if (xhttp.readyState == 4 && xhttp.status == 200) {
      console.log("Valor enviado al servidor con éxito");
    }
  };

  xhttp.open("GET", url, true);
  xhttp.send();
}

function printDetener() {
  boton.innerHTML = "Detener"
}




