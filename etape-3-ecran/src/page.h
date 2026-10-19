// Page web de commande, servie sur http://lecteur.local
// Elle interroge /etat toutes les secondes et envoie les commandes par /cmd.
#pragma once

static const char PAGE_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="fr"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Lecteur audio</title>
<style>
body{font-family:system-ui,sans-serif;background:#111;color:#eee;margin:0;padding:16px;max-width:520px;margin:auto}
h1{font-size:1.3em;margin:0 0 12px}
.carte{background:#1e1e1e;border-radius:12px;padding:14px;margin-bottom:14px}
#titre{font-size:1.15em;font-weight:600;min-height:1.4em;word-break:break-word}
#infos{color:#aaa;font-size:.9em;margin-top:4px}
.commandes{display:flex;gap:10px;justify-content:center;margin:14px 0}
button{background:#2d6cdf;color:#fff;border:0;border-radius:10px;font-size:1.3em;padding:10px 18px;cursor:pointer}
button.petit{font-size:.9em;padding:8px 12px;background:#333}
button.actif{background:#e08a1e}
input[type=range]{width:100%}
.onglets{display:flex;gap:8px;margin-bottom:10px}
ul{list-style:none;padding:0;margin:0;max-height:50vh;overflow:auto}
li{padding:9px 6px;border-bottom:1px solid #2a2a2a;cursor:pointer}
li.encours{color:#e08a1e;font-weight:600}
progress{width:100%;height:6px}
</style></head><body>
<h1>Lecteur audio</h1>
<div class="carte">
  <div id="titre">…</div>
  <div id="infos"></div>
  <progress id="avance" value="0" max="1"></progress>
  <div class="commandes">
    <button onclick="cmd('precedent')">&#9198;</button>
    <button id="bLecture" onclick="cmd('lecture')">&#9199;</button>
    <button onclick="cmd('suivant')">&#9197;</button>
  </div>
  <label>Volume <span id="vVal"></span></label>
  <input type="range" id="volume" min="0" max="21" oninput="volume(this.value)">
  <div class="commandes">
    <button class="petit" id="bAlea" onclick="cmd('aleatoire')">Aléatoire</button>
    <button class="petit" id="bMinu" onclick="cmd('minuteur')">Minuteur</button>
  </div>
</div>
<div class="carte">
  <div class="onglets">
    <button class="petit" id="oSd" onclick="onglet('sd')">Carte SD</button>
    <button class="petit" id="oRadio" onclick="onglet('radio')">Webradios</button>
  </div>
  <ul id="liste"></ul>
</div>
<script>
let listes={sd:[],radio:[]}, vue='sd', etat={}, glisse=0;
function cmd(a){fetch('/cmd?a='+a).then(maj)}
function volume(v){glisse=Date.now();document.getElementById('vVal').textContent=v;fetch('/cmd?a=volume&v='+v)}
function jouer(type,i){fetch('/cmd?a='+type+'&n='+i).then(maj)}
function onglet(o){vue=o;dessiner()}
function hms(s){s=s|0;return (s/60|0)+':'+String(s%60).padStart(2,'0')}
function dessiner(){
  document.getElementById('oSd').className='petit'+(vue=='sd'?' actif':'');
  document.getElementById('oRadio').className='petit'+(vue=='radio'?' actif':'');
  const ul=document.getElementById('liste');ul.innerHTML='';
  listes[vue].forEach((nom,i)=>{const li=document.createElement('li');li.textContent=nom;
    if(etat.mode==vue&&etat.index==i)li.className='encours';
    li.onclick=()=>jouer(vue,i);ul.appendChild(li)});
}
function maj(){fetch('/etat').then(r=>r.json()).then(e=>{
  const change=e.mode!=etat.mode||e.index!=etat.index;etat=e;
  document.getElementById('titre').textContent=e.titre||e.nom||'—';
  let infos=e.mode=='radio'?'Webradio : '+e.nom:'Piste '+(e.index+1)+'/'+listes.sd.length+(e.duree?' · '+hms(e.temps)+' / '+hms(e.duree):'');
  if(e.pause)infos+=' · en pause';
  if(e.minuteur)infos+=' · arrêt dans '+e.minuteur+' min';
  document.getElementById('bMinu').className='petit'+(e.minuteur?' actif':'');
  document.getElementById('infos').textContent=infos;
  const p=document.getElementById('avance');p.max=e.duree||1;p.value=e.mode=='radio'?0:e.temps;
  if(Date.now()-glisse>2000){document.getElementById('volume').value=e.volume;document.getElementById('vVal').textContent=e.volume}
  document.getElementById('bAlea').className='petit'+(e.aleatoire?' actif':'');
  if(change)dessiner();
}).catch(()=>{})}
Promise.all([fetch('/liste').then(r=>r.json()),fetch('/radios').then(r=>r.json())]).then(([sd,radio])=>{
  listes.sd=sd;listes.radio=radio;maj();dessiner()});
setInterval(maj,1000);
</script></body></html>
)rawliteral";
