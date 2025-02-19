/*********************************************************************
* ecef_path_generator.js                                             *
* (https://github.com/fpdf-easytable/ECEF_path_generator)            *
*                                                                    *
* Version: 2.0                                                       *
* Date:    28-01-2022                                                *
* Author:  Dan Machado                                               *
* Require  raphaeljs v2.2.1                                          *
**********************************************************************/
function roundNumber(x, p){
	var d=2;
	if(typeof p==='number') {d=p;}	
	return (Math.round(x* Math.pow(10, d))/Math.pow(10,d)).toFixed(p);
}
//##################################################################
function distance2DPoints(x1, y1, x2, y2){return Math.sqrt(Math.pow(x1-x2, 2) + Math.pow(y1-y2, 2));}
//####################################################################
var mkVect2D=(function(){
	var Vec2D={
		x:0,y:0,
		setPositionAt:function(x, y){
			this.x=x;
			this.y=y;
		},
	};
	return function(x, y){
		var vect=Object.create(Vec2D);
		vect.setPositionAt(x, y);
		return vect;
	}
})();
//####################################################################
Conversion={
	a : 1/60,	b: 1/3600,
	kmToDDNorth:(function(){
		const dgK=110.574;	const mnK=dgK/60;	const scK=dgK/3600;
		return function(km){
			var d=km/dgK;var degs=Math.floor(d);var m=(km-degs*dgK);
			var mins=Math.floor(m/mnK);var secs=m-mins*mnK;
			return degs+(mins*60+secs/scK)/3600;
		}
	})(),		
	kmToDDEast:(function(){
		var MPI=Math.PI/180.0;	var dgK, mnK, scK, d, m;
		return function(lat, km){
			dgK=111.321*Math.cos(MPI*lat);mnK=dgK/60;scK=dgK/3600;d=km/dgK;var degs=Math.floor(d);
			m=(km-degs*dgK);var mins=Math.floor(m/mnK);var secs=m-mins*mnK;
			return degs+(mins*60+secs/scK)/3600;
		}
	})(),
};
//####################################################################
(function(){
	const baseFactor=206;const initialZoom=14.25;const zoomDelta=0.25;

	Settings={	
		alt:30,
		currentZoom:initialZoom,
		setAltitude:function(centerAlt){this.alt=Number(centerAlt);},
		initialZoom:function(){return initialZoom;},
		zoomDelta:function(){return zoomDelta;},
		updateZoom:function(deltaZoom){currentZoom+=deltaZoom;},		
	};
	return Settings;
})();

var map = L.map('map',{      
		minZoom: 0,
		zoomSnap: 0,
		zoomDelta: 0.25,
		zoomControl:false,
});
//####################################################################
(function(){
	var container=document.getElementById('container');var diff=window.innerHeight;
	container.style.height=document.body.clientHeight+'px';
	container.style.height=window.innerHeight+'px';
	document.getElementById('canvasContainer').style.height=diff+'px';document.getElementById('canvas').style.height=diff+'px';
	document.getElementById('map').style.height=diff+'px';document.getElementById('left_pannel').style.height=diff+'px';

	var canvasWidth=document.getElementById('canvas').clientWidth;var canvasHeight=diff;const baseFactor=206;const baseZoom=14.25;
	globalSettings={
		canvas:document.getElementById('canvas'),
		baseZ:baseZoom,
		grid:[],
		Height:canvasHeight,
		Width:canvasWidth,
		scaleFactor:baseFactor,
		getScaleFactor:function(){
			return this.scaleFactor
		},
		gridSettings:{stroke: "#2929a3",'stroke-width':0.2, opacity: 0.6},
		paper:Raphael("canvas", canvasWidth, canvasHeight),
		wMtPx:0.0,
		hMtPx:0.0,
		mkGrid:function(){
			if(this.grid.length>0){
				for(var i=0; i<this.grid.length; i++){
					this.grid[i].remove();
				}
				this.grid.length=0;
			}

			var widthM=map.distance(map.containerPointToLatLng(L.point(0, 0)), map.containerPointToLatLng(L.point(this.Width, 0)));
			this.wMtPx=widthM/this.Width;
			var heightM=map.distance(map.containerPointToLatLng(L.point(0, 0)), map.containerPointToLatLng(L.point(0, this.Height)));
			this.hMtPx=heightM/this.Height;
			var hh=1000/this.hMtPx;
			var hK=Math.floor(this.Height/hh);
			for(var i=0; i<hK; i++){
				this.grid.push(this.paper.path('M0,'+(i+1)*hh+'L'+this.Width+','+(i+1)*hh).attr(this.gridSettings));
			}
			var ww=1000/this.wMtPx;
			var wK=Math.floor(this.Width/ww);
			this.scaleFactor=Math.floor(ww);
			this.scFacInv=1.0/this.scaleFactor;
			for(var i=0; i<wK; i++){
				this.grid.push(this.paper.path('M'+(i+1)*ww+',0L'+(i+1)*ww+','+this.Height).attr(this.gridSettings));
			}
		},
		mtsToPxs:function(mts){return mts*0.001*this.scaleFactor;},
		kmsToPxs:function(kms){return kms*this.scaleFactor;},
		pixelsToKms:function(pxD){return pxD*this.scFacInv;},
		pixelsToMts:function(pxD){return this.pixelsToKms(pxD)*1000.0;},
		containerPointToLatLng:function(pxX, pxY){
			var mapCenter=map.getCenter();			
			var lat=mapCenter.lat+Conversion.kmToDDNorth((0.5*this.Height-pxY)*this.scFacInv);
			var lng=mapCenter.lng+Conversion.kmToDDEast(lat, (pxX-0.5*this.Width)*this.scFacInv);
			return {'lat':lat, 'lng':lng};
 		},
		containerCenter:function(){
			return mkVect2D(0.5*this.Width, 0.5*this.Height);},
		latLngToPixelCoordinates:function(latLng){
			var R=this.mtsToPxs(map.distance(latLng, map.getCenter()));
			var center=this.containerCenter();
			var pos=map.latLngToContainerPoint(latLng);
			var r=distance2DPoints(center.x, center.y, pos.x, pos.y);
			var factor=R/r;
			var x=factor*Math.abs(center.x-pos.x);
			var y=factor*Math.abs(center.y-pos.y);
			var pixelsCoordinates={x:(center.x+x), y:(center.y+y)};
			if(pos.x<=center.x){
				pixelsCoordinates.x=center.x-x;
			}
			if(pos.y<=center.y){
				pixelsCoordinates.y=center.y-y;
			}
			return pixelsCoordinates;},
		latLngToMtsCoordinates:function(latLng){
			var R=map.distance(latLng, map.getCenter());var center=this.containerCenter();
			var pos=map.latLngToContainerPoint(latLng);var r=distance2DPoints(center.x, center.y, pos.x, pos.y);
			var factor=R/r;var px=factor*(pos.x-center.x);var py=factor*(center.y-pos.y);
			return {x:px, y:py};},
		mtsCooPxs:function(x, y, cbk){cbk(this.mtsToPxs(x)+0.5*this.Width, -1*this.mtsToPxs(y)+0.5*this.Height);},
		mtsCooPxs2:function(x, y){return {transform: 't'+(this.mtsToPxs(x)+0.5*this.Width)+','+(-1*this.mtsToPxs(y)+0.5*this.Height)};},
	};
	return globalSettings;
})();
//####################################################################
// add the OpenStreetMap tiles
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
	attribution: '&copy; <a href="https://openstreetmap.org/copyright">OpenStreetMap contributors</a>'
}).addTo(map);

L.control.scale({maxWidth:300}).addTo(map);
//####################################################################
var mkObjectEvt=(function(){
	var objectEvent={
		addHandler:function(evt,ck){
			this.handlers[evt]=this.handlers[evt] || [];this.handlers[evt].push(ck);
		},
		fire:function(evt){
			for(var i in this.handlers[evt]){
				this.handlers[evt][i].apply(null, Array.prototype.slice.call(arguments, 1));
			}
		},
	};
	return function(){
		var module=Object.create(objectEvent);module.handlers={};
		return module;
	}
})();
//####################################################################
(function(){
	controlModule=mkObjectEvt();
	controlModule.pauseButton=document.getElementById('pauseButton');
	controlModule.init=function(){
		var zoomVal=globalSettings.baseZ;var zoomStep=0.25;
		controlModule.setZoom=function(zoomLevel){
			zoomVal=zoomStep*Math.floor(zoomLevel/zoomStep);
			if(zoomVal<12){
				zoomVal=globalSettings.baseZ;
			}
			map.setZoom(zoomVal);
		};
		document.getElementById('zoom_less').addEventListener('click', function(){
			if(zoomVal-0.25>=11){
				zoomVal-=0.25;
				map.setZoom(zoomVal);
			}
		});		
		document.getElementById('zoom_more').addEventListener('click', function(){
			if(zoomVal+0.25<=17.0){
				zoomVal+=0.25;
				map.setZoom(zoomVal);
			}
		});
		this.pauseButton.addEventListener('click', function () {
			controlModule.fire('pause');controlModule.pauseButton.style.display='none';
			controlModule.playButton.style.display='block';
		});
	};
	return controlModule;
})();
//####################################################################
var ZoomViewer = L.Control.extend({
	onAdd: function(){
		var container= L.DomUtil.create('div');
		var gauge = L.DomUtil.create('div');
		container.style.width = '200px';
		container.style.background = 'rgba(255,255,255,0.5)';
		container.style.tespeedTimeAlign = 'left';
		map.on('zoomstart zoom zoomend', function(ev){
			var zp=map.getZoom();
			gauge.innerHTML = 'Zoom level: ' + zp;
			controlModule.fire('scaleChange', zp-globalSettings.baseZ);
		})
		container.appendChild(gauge);
		return container;
	}
});
(new ZoomViewer).addTo(map);
//####################################################################
var mkPOI=(function(){
	var objectPOI={
		geoPosition2D:null,POI:null,lat:'lat',lng:0.0,
		setAttr:function(attributes){this.POI.attr(attributes);},
		pointToLatLng:function(x, y){
			var latLng=globalSettings.containerPointToLatLng(x, y);
			this.lat=latLng.lat;this.lng=latLng.lng;
		},
		setLatLng:function(latLng){
			this.lat=latLng.lat;this.lng=latLng.lng;
			var pos=globalSettings.latLngToPixelCoordinates(latLng);
			this.POI.attr({transform: 't'+pos.x+','+pos.y});this.POI.toFront();
			this.geoPosition2D=globalSettings.latLngToMtsCoordinates({'lat':this.lat, 'lng':this.lng});
		},
		onZoom:function(){
			var pos=globalSettings.latLngToPixelCoordinates({'lat':this.lat, 'lng':this.lng});
			this.POI.attr({transform: 't'+pos.x+','+pos.y});this.POI.toFront();
		},
		getGeoX:function(){return this.geoPosition2D.x;},getGeoY:function(){return this.geoPosition2D.y;},
		remove:function(){this.POI.remove();},
	};
	return function(x, y, attributes, radius){
		var module=Object.create(objectPOI);var poiR=2;var att={stroke:'none'};
		if(typeof attributes!=="undefined" && typeof attributes=='object'){
			att=attributes;
			if(typeof radius!=="undefined"){
				poiR=radius;
			} 
		}
		module.POI=globalSettings.paper.circle(0, 0, poiR);module.POI.attr(att);
		return module;
	}
})();
//####################################################################
(function(){
	Trail={
		set:[],
		looping:function(doSomething){
			for(var i=0; i<this.set.length; i++){
				doSomething(this.set[i]);
			}
		},
		onZoom:function(){
			this.looping(function(shape){
				shape.onZoom();
			});
			this.mover.attr(globalSettings.mtsCooPxs2(this.currentGeoPos.x, this.currentGeoPos.y));
			this.mover.toFront();
		},
		reset:function(){
			this.looping(function(shape){
				shape.remove();
			});
			this.set.length=0;this.trailLength=0.0;this.segmentNum=0;this.distanceMt=0.0;
		},
		trailPOIcolour:"#0000ff",trailLength:0.0,
		prevLatLng:null,mover:null,
		init:function(){
			this.mover=globalSettings.paper.circle(0, 0, 2);this.mover.attr({stroke:'none', fill: "blue"});this.mover.attr({transform: 't-100,-100'});
		},
		addPointLatLng:function(latLng){
			this.set.push(mkPOI(0, 0, {stroke:'none', fill: this.trailPOIcolour}));var k=this.set.length-1;
			this.set[k].setLatLng(latLng);
			if(k>0){
				this.trailLength+=map.distance(latLng, this.prevLatLng);
			}
			this.prevLatLng=latLng;
		},
		getTrailLengthKm:function(){
			document.getElementById("Tdistance").innerHTML=roundNumber(this.trailLength*0.001, 2);
		},
		currentGeoPos:mkVect2D(0, 0),segmentNum:0,distanceMt:0,deltaGeo:mkVect2D(0, 0),
	};
	return Trail;
})();
//####################################################################
var drawer={
	offSetX:-1.63,offSetY:53.2672,socket:null,
	init:function(){
		this.socket = new WebSocket("ws://127.0.0.1:22002");
		this.socket.onerror = function(event) {
			console.error("WebSocket error observed:", event);
			alert("unable to connect websocket via: 127.0.0.1:22002");
		};
		this.socket.onopen = function (event) {
			console.log('Ready');
		};
		this.socket.onmessage = function (event) {			
			Trail.addPointLatLng(JSON.parse(event.data));
			Trail.getTrailLengthKm();
		};
		Trail.init();
		controlModule.init();
		controlModule.addHandler('pause', function(){
			drawer.halt();
		});
		controlModule.addHandler('scaleChange', function(x){			
			if(drawer.mapReady()){
				globalSettings.mkGrid();
				Trail.onZoom();
			}
		});
	},
	isMapReady:false,
	mapReady:function(){
		return this.isMapReady;
	},
	setCoordinates:function(){
		var lat=53.2672, lng=-1.63;
		this.isMapReady=true;this.offSetX=Number(lng);this.offSetY=Number(lat);
		map.setView([Number(lat), Number(lng)], 13.50);globalSettings.mkGrid();
		document.cookie='longitude='+this.offSetX;document.cookie='latitude='+this.offSetY;
	},		
};
//####################################################################
drawer.init();
controlModule.setZoom(13.50);
//####################################################################
window.addEventListener("DOMContentLoaded", function(){
	window.scrollTo(0, 0);
	(function(){
		drawer.setCoordinates();
	})();
})
