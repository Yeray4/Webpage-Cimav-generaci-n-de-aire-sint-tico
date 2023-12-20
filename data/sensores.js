//Al cargar la pagina se envia un valor de control al esp32 para saber en que pagina se encuentra y tener una variable de control
document.addEventListener("DOMContentLoaded", function() {
   var valueToSend = "100"; 
   var url = "/ubi?ubi=" + encodeURIComponent(valueToSend);
   
   var xhttp = new XMLHttpRequest();
   
   xhttp.onreadystatechange = function() {
     if (xhttp.readyState == 4 && xhttp.status == 200) {
       console.log("Valor enviado al servidor con éxito");
     }
   };
   
   xhttp.open("GET", url, true);
   xhttp.send();
 
 });



 var socket = new WebSocket("ws://" + window.location.hostname + ":80/ws");
  
 socket.onmessage = function(event) {
   var data = event.data;
   var values = data.split(",");
   
   // Aquí puedes procesar los valores recibidos como desees
   var t1 = parseFloat(values[0]);
   var t2 = parseFloat(values[1]);
   var t3 = parseFloat(values[2]);
   var t4 = parseFloat(values[3]);
   var t5 = parseFloat(values[4]);
   var h1 = parseFloat(values[5]);
   var h2 = parseFloat(values[6]);
   var h3 = parseFloat(values[7]);
   var h4 = parseFloat(values[8]);
   var h5 = parseFloat(values[9]);
   var dstemp = parseFloat(values[10]);
   var fhviento = parseFloat(values[11]);
   
   // Actualizar la interfaz con los valores de temperatura recividos
   document.getElementById("valort1").textContent = t1;
   document.getElementById("valort2").textContent = t2;
   document.getElementById("valort3").textContent = t3;
   document.getElementById("valort4").textContent = t4;
   document.getElementById("valort5").textContent = t5;
   document.getElementById("valorRH1").textContent = h1;
   document.getElementById("valorRH2").textContent = h2;
   document.getElementById("valorRH3").textContent = h3;
   document.getElementById("valorRH4").textContent = h4;
   document.getElementById("valorRH5").textContent = h5;
   ocument.getElementById("dstemp").textContent = dstemp;
   document.getElementById("valorfhv").textContent = fhviento;
 };












// Referencia : https://www.luisllamas.es/comunicar-una-pagina-web-con-un-esp8266-con-peticiones-ajax/
/* var valorSpan = document.getElementById("valort1");
var valorSpan2 = document.getElementById("valorRH1");

function updateData(valor)
{
  valorSpan.innerHTML = valor; 
}

function updateData2(valor)
{
  valorSpan2.innerHTML = valor; 
}

function ajaxCall() {
    var xmlhttp = new XMLHttpRequest();
    var xmlhttp2 = new XMLHttpRequest();

    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == XMLHttpRequest.DONE) {
           if (xmlhttp.status == 200) {
              updateData(xmlhttp.responseText);
           }
           else {
              console.log('error', xmlhttp);
           }
        }
    };

    xmlhttp2.onreadystatechange = function() {
        if (xmlhttp2.readyState == XMLHttpRequest.DONE) {
           if (xmlhttp2.status == 200) {
              updateData2(xmlhttp2.responseText);
           }
           else {
              console.log('error2', xmlhttp2);
           }
        }
    };

    xmlhttp.open("GET", "GetTemp_Pronos", true);
    xmlhttp.send();
    xmlhttp2.open("GET", "GetHum_Pronos", true);
    xmlhttp2.send();
}

(function scheduleAjax() {
    ajaxCall();
    setTimeout(scheduleAjax, 1000);
})(); */