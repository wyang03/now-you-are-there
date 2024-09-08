(
Server.killAll;
~cleanup = {
	Buffer.freeAll;
	Window.closeAll;
	ServerBoot.removeAll;
	ServerTree.removeAll;
	CmdPeriod.removeAll;
};
ServerQuit.add(~cleanup);
~cleanup.value;
s.options.memSize_(2.pow(21)).numWireBufs_(256*8).maxNodes_(8192)
.numInputBusChannels_(4).numOutputBusChannels_(4);

s.waitForBoot({
	~monoEnc = FoaEncoderMatrix.newDirection;
	~stereoEnc = FoaEncoderMatrix.newStereo(pi/4);
	~uhjDec = FoaDecoderKernel.newUHJ;  //stereo speakers
	// ~binDec = FoaDecoderKernel.newSpherical;  //binaural
	~quadDec = FoaDecoderMatrix.newQuad(pi/4, 'energy'); //quad for gallery

	s.sync;

	//set up any decode___________________________
	~foaBus = CtkAudio.new(4, server: s);
	~outBus = ~foaBus; //stereo or quad
	// ~outBus = 0; //direct out


	//decode________________
	~decodeOut = CtkSynthDef(\decodeOut, {|inBus, decoder, outBus=0|
		var inSig, outSig;
		inSig = In.ar(inBus, 4);
		// decoder = ~uhjDec;  //uhjj stereo
		decoder = ~quadDec; //quad
		outSig = FoaDecode.ar(inSig, decoder);
		Out.ar(outBus, outSig)
	});

	~prep = {
		// ~stereoDec.free; ~foaBus.free;
		// ~foaBus = CtkAudio.new(4, server: s);
		// ~stereoDec = FoaDecoderKernel.newUHJ;
		~decodeOut.note.inBus_(~foaBus).play;
		// Window.closeAll;s.plotTree;
		~mlOnsets = 0; ~mlAmp =0; ~mlFreq = 0; ~mlEntropy = 0; ~mlFlat = 0; ~mlCentro = 0;~ampThresh = 0.1;
		~ampIn =  0; ~onsetsIn = 1; ~ampThresh = 0.1; ~freqIn = 2; ~flatIn = 3; ~entropyIn = 0; ~centroIn = 0; ~trate = 30; //
	};
	CmdPeriod.add({ServerTree.removeAll; ServerTree.add(~prep)});
	CmdPeriod.run;
	//________________________________________________________________


	//load buffers_____________________________________________
	~folderPath = "foa/23_ recall_short.wav".resolveRelative;  //one foa file - for installation
	~foaBufs = []; ~bufNames = [];
	~entries = PathName(~folderPath).entries;
	// PathName(~folderPath).entries.size
	if(~entries.size == 0){
		~foaBufs = ~foaBufs.add(CtkBuffer(~folderPath).load);
		~wBuf = CtkBuffer(~folderPath, channels: 0).load
	}{
		PathName(~folderPath).filesDo{|thisPath, i|
			thisPath.fullPath.basename.postln;
			~foaBufs = ~foaBufs.add(CtkBuffer(thisPath.fullPath).load);
			~bufNames  = ~bufNames .add(thisPath.fullPath.basename);
			if(i == 23){~wBuf = CtkBuffer(thisPath.fullPath, channels: 0).load};
		};
		// ~wBuf = CtkBuffer("foa/23_ recall_short.wav".resolveRelative).load;
		~bufferInfo = {~bufNames.do(_.postln); nil};
	};


	~voiceFolder = "/Applications/of_v0.12.0/apps/myApps/prototype_1/sc code/foa_voice/";
	~voiceBufs = [];
	PathName(~voiceFolder).filesDo{|thisPath, i|
		thisPath.fullPath.basename.postln;
		~voiceBufs = ~voiceBufs.add(CtkBuffer(thisPath.fullPath).load);
	};


	s.sync;
	s.plotTree;



	//synthDefs__________________________________________________________________

	~tone = CtkSynthDef.new(\tone, {|outBus = 0, noteDur = 5, ris=0.1, dec=0.1, gate=0, freq =300, rotAngle=0, tilAngle=0, tumAngle=0, gain = 0.0|
		var sig, env, outSig;
		env = Env.new([0, 1, 0.8, 0], [ris, 1 - (ris+dec), dec]).timeScale(noteDur).kr(2, gate: gate);
		// sig = SinOsc.ar(freq.lag(0.02) + PinkNoise.ar(0.1));
		sig = PinkNoise.ar;
		sig = FoaEncode.ar(sig, ~monoEnc);
		sig = FoaRTT.ar(sig, rotAngle, tilAngle, tumAngle);
		outSig = sig * env * gain.dbamp;
		Out.ar(outBus, outSig);
	});

	~plb = CtkSynthDef(\plb, {|outBus = 0, gate = 1, noteDur =5, ris=0.1, dec = 0.1, rate = 1, startPos = 0, buffer = 0, loop = 1, gain = 0.0, rotAngle=0, tilAngle=0, tumAngle=0, hpFreq = 80|
		var sig, env, outSig;
		env = Env.new([0, 1, 0.8, 0], [ris, 1 - (ris+dec), dec]).timeScale(noteDur).kr(2, gate: gate);
		startPos = startPos * BufFrames.kr(buffer);
		sig = PlayBuf.ar(4, buffer, BufRateScale.kr(buffer) * rate, trigger: 0, startPos: startPos, loop: loop, doneAction: 2);
		sig = FoaRTT.ar(sig, rotAngle, tilAngle, tumAngle);
		outSig = sig * env * gain.dbamp;
		outSig = HPF.ar(outSig, hpFreq);
		Out.ar(outBus, outSig);
	});

	~noisePerc =CtkSynthDef(\noisePerc, {|outBus = 0, freq = 440, pan=0, noteDur = 5, gain = -12, gate =1, ris=0.1, dec=0.1, decTime = 1, rq = 0.1, rate = 1, angle = 0.33pi, theta = 0, phi = 0|
		var freqs, amps, ringtimes;
		var sig, env, outSig;

		// env = Env.new([0, 1, 0.8, 0], [ris, 1 - (ris+dec), dec]).timeScale(noteDur).kr(2, gate: gate);
		env = Env.asr.timeScale(noteDur).kr(0, gate);
		freqs = Control.names([\freqRatios]).kr(Array.rand(8, 1, 20.0)).lag(0.1) * freq;
		ringtimes = Control.names([\ringtimes]).kr(Array.fill(8, 0.5));
		amps = Control.names([\amps]).kr(Array.fill(8, 0.1));

		sig = DynKlank.ar(`[freqs, amps, ringtimes], PinkNoise.ar );
		sig = Decay2.ar(Dust.ar(rate), decayTime: decTime, mul: sig);
		sig = FoaEncode.ar(sig, FoaEncoderMatrix.newOmni);
		sig = FoaPush.ar(sig, angle, theta, phi);
		outSig = sig * env * gain.dbamp;
		outSig = HPF.ar(outSig, 40);
		Out.ar(outBus, outSig * 0.025);
	});

	~sustainTex = CtkSynthDef(\sustainTex, {|outBus=0, ris=0.1, dec=0.1, gate = 1, gain=0.0, noteDur=10, maxDelayTime=0.2, decayTime=3, lfnFreq=0.1, delayTime = 0.04, angle = 0.33pi, theta=0, phi=0.0, rate=1, minDev = 0.8, maxDev = 1.2, freq = 40|
		var env, sig, rtt, foaSig, outSig;
		var amp = (gain-12).dbamp;
		var freqs, amps, ringtimes;
		// env = EnvGen.kr(Env([0, 1, 0.8, 0], [ris, 1-(ris+dec), dec]), timeScale: noteDur, doneAction: 0, gate: gate);
		env = Env.asr(ris, 1 - (ris+dec), dec).timeScale(noteDur).kr(0, gate);
		freqs = Control.names([\freqRatios]).kr(Array.rand(5, 1, 20.0)).lag(0.1) * freq.lag(0.01);
		ringtimes = Control.names([\ringtimes]).kr(Array.fill(5, 0.5));
		amps = Control.names([\amps]).kr(Array.fill(5, 0.1));
		// sig = DynKlang.ar(`[freqs* LFNoise2.kr((lfnFreq/4)!5).range(minDev, maxDev) , amps , ringtimes]);
		sig = DynKlank.ar(`[freqs * LFNoise2.kr((lfnFreq)!5).range(minDev, maxDev), amps, ringtimes], PinkNoise.ar);
		// sig = Decay2.ar(Dust.ar(rate), mul: sig);
		3.do{sig = AllpassN.ar(sig, maxDelayTime, [delayTime, 0.04.rand] * LFNoise2.kr(lfnFreq/4).range(0.8, 1.2), decayTime)};//mono -> stereo
		// foaSig = FoaEncode.ar(sig.sum, ~monoEnc);
		foaSig = FoaEncode.ar(sig, ~stereoEnc);
		// foaSig = FoaPush.ar(foaSig, angle, theta, phi);
		rtt = LFNoise2.kr(0.1!3);
		foaSig = FoaRTT.ar(foaSig, rtt[0].range(-pi, pi), rtt[1].range(-pi, pi), rtt[2].range(-pi, pi));
		outSig = foaSig * env * amp;
		Out.ar(outBus, outSig);
	});

	~grain = CtkSynthDef(\grainBuf, {|gain=0,buffer=0, gate = 0, freq = 10, grainDur = 0.05, grainRate = 1, indx = 0.2, inBus = 0, outBus = 0, angle = 0.33pi, theta = 0, phi = 0 |
		var sig, outSig;
		var env = Env.adsr().kr(0, gate);
		// sig = GrainBuf.ar(1, Impulse.ar(freq), MouseX.kr(0.1, 1.0).wrap(0.1, 1).poll, buffer, grainRate, MouseY.kr(0, 1).clip(0, 1) );
		sig = GrainBuf.ar(1, Dust.ar(freq), grainDur, buffer, grainRate, indx);

		// sig = PinkNoise.ar(0.1);
		sig = FoaEncode.ar(sig, ~monoEnc);
		sig = FoaPush.ar(sig, angle, theta, phi);
		// sig =FoaRotate.ar(sig, theta);
		outSig = sig * env * gain.dbamp;
		Out.ar(outBus, outSig);
	});

	////////Machine Listening///////Sending to ~micListening////////Map the Inputs!!!

	~oscTrig= CtkSynthDef(\oscTrig, {|trate = 10, ampThresh=0.1, onsetsIn=0, ampIn=0, freqIn=0, entropyIn=0, flatIn = 0, centroIn=0, relaxtime=0.1|
		var onsetsSig, ampSig, freqSig, entropySig, flatSig, centroSig;
		var onsetsChain, entropyChain, flatChain, centroChain;
		var onsets, amp, freq, hasFreq, entropy, flatness, centroid;
		var out;

		onsetsSig = SoundIn.ar(onsetsIn); onsetsChain = FFT(LocalBuf(2048), onsetsSig);
		ampSig = SoundIn.ar(ampIn); //Amplitude.kr doesn't require FFT
		freqSig = SoundIn.ar(freqIn); //Pitch.kr doesn't require FFT
		entropySig = SoundIn.ar(entropyIn); entropyChain = FFT(LocalBuf(2048), entropySig);
		flatSig = SoundIn.ar(flatIn); flatChain = FFT(LocalBuf(2048), flatSig);
		centroSig = SoundIn.ar(centroIn); centroChain = FFT(LocalBuf(2048), centroSig);
		// #onsetsChain, ampChain, freqChain, entropyChain, flatChain, centroChain = {FFT(LocalBuf(2048), inSig)}!6;

		onsets = Onsets.kr(onsetsChain, ampThresh, 'magsum', relaxtime: relaxtime);//\power, \magsum, \mlk, \rcomplex
		amp = Amplitude.kr(ampSig, releaseTime: 0.5);
		#freq, hasFreq = Pitch.kr(freqSig, clar: 0); //clar: 1
		entropy = SpectralEntropy.kr(entropyChain, 2048);  // complexity of sound: 0.7 - 125

		flatness = SpecFlatness.kr(flatChain);  // noisiness: 0 - 1
		centroid = SpecCentroid.kr(centroChain);  //brightness: 300 - 11000, mean frequency of the spectrum in HZ

		SendReply.kr(Impulse.kr(trate), '/mic',
			[onsets, amp, freq,entropy,flatness,centroid], replyID: -1
		);
		// //only for testing purpose
		// out = SelectXFocus.ar(onsets, [Silence.ar, SinOsc.ar([freq, freq + 1], 0, EnvGen.kr(Env.adsr(0.01, 0.1, 0.5,1.0), onsets))*amp], 1);//Trig.kr(hasFreq)
		// Out.ar(0, out)
	});

	////////Functions///////////////OSCFunc ////////////////and Composition Func////////
	////++++////++++////++++////++++////++++////++++////++++////++++////++++////++++////
	////////OSCFunc to Listen to ~oscTrig////////from prototype_4////////

	~micListen = OSCFunc({|msg|
		//[3]-onsets, [4]-amp, [5]-freq, [6]-entropy, [7]-flatness, [8]-centroid
		// msg[3.. 6].postln;

		~mlOnsets = msg[3];
		~mlAmp = msg[4];
		~mlFreq = msg[5];
		~mlEntropy = msg[6];
		~mlFlat = msg[7];
		~mlCentro = msg[8];
		// [~mlOnsets, ~mlAmp].postln;
	}, '/mic', s.addr);



	s.sync;

	//OSC Communication between OF and SC__________________________________________________________________
	~ofSendPort = 12345;
	~ofOscPath = '/ofParams'; ~scOscPath = '/scParams';
	~ofLisPort = 12344;  //SC send to OF's LISPORT
	~ofOSC = NetAddr.new("127.0.0.1", ~ofLisPort); //from touchOsc,'s local IP address
	~ofSendRate = 30;

	//receiving from OF
	//numVerts, centroidX, centroidY, nBlobs, blobPts, avgRepelFrc, avgVel, smoothDist, zDev
	~ofOscTestFunc = OSCFunc({|msg, time, addr, recvPort|
		msg.postln;
	}, ~ofOscPath, nil, ~ofSendPort);
	~ofOscTestFunc.disable;

	//sending to OF
	~micListen.enable;
	~note = ~oscTrig.note.ampIn_(~ampIn).onsetsIn_(~onsetsIn).ampThresh_(0.002).freqIn_(~freqIn).flatIn_(~flatIn).entropyIn_(~entropyIn).
	centroIn_(~centroIn).trate_(~trate).play;
	~sendOscR =Routine({
		loop{
			// ~ofOSC.sendMsg(~scOscPath, ~mlAmp, ~mlFreq, ~mlEntropy);
			~ofOSC.sendMsg(~scOscPath, ~mlAmp);

			(~ofSendRate.reciprocal).wait;
		}
	});
	~sendOscR.play;

	//sound_____________________________

	~grainNote = ~grain.note.buffer_(~wBuf).outBus_(~outBus).gate_(0).grainDur_(CtkControl.lfo(LFNoise2, 0.05, 0.5)).play;
	~texNote = ~sustainTex.note.outBus_(~outBus).noteDur_(10).rate_(4).gain_(-24.0).gate_(1).play;

	~listen = OSCdef.new(\listener, {|msg, time, addr, recvPort|
		var numVerts, centroidX, centroidY, numBlobs, numBlobPts, avgRepelFrc, avgVel, dist, zDev;
		var minNumVerts = 300, maxNumVerts = 800;
		var minCentroidX=200, maxCentroidX=1000, minCentroidY=600, maxCentroidY=900;
		var minNumBlobs, maxNumBlobs;
		var minNumBlobPts, maxNumBlobPts;
		var minAvgRepelFrc = 200, maxAvgRepelFrc = 800;
		var minAvgVel=1.0, maxAvgVel=3.5;
		var minDist = 1100, maxDist = 1500;

		numVerts = msg[1]; centroidX = msg[2]; centroidY = msg[3]; numBlobs = msg[4];
		numBlobPts = msg[5]; avgRepelFrc = msg[6]; avgVel = msg[7]; dist = msg[8]; zDev = msg[9];



		//plb____________________________________________
		// if(((msg[1] < minNumVerts) && (msg[1] > 100)) || (msg[1]>maxNumVerts)){
		if((numVerts> (minNumVerts * 2)) && (numVerts<maxNumVerts)){
			~plb.note.buffer_(~voiceBufs.choose).outBus_(~outBus).noteDur_(rrand(0.5, 2.0)).rotAngle_(rrand(-pi, pi))
			.rate_(linlin(minNumVerts, maxNumVerts, -1.0, 1.0)).startPos_(rrand(0.0, 1.0))
			.gain_(linlin(minNumVerts, maxNumVerts, -9.0, 3.0)).gate_(1).loop_(1)
			.play
		}{~plb.note.gate_(0)};


		//sustain_____________________________________
		~texNote.freq_(numVerts.linlin(100, maxNumVerts, 20, 40)).gain_(centroidY.linlin(minCentroidY, maxCentroidY, -45.0, -36.0)).rate_(msg[4].linlin(200, 400, 0.1, 0.4));


		//grain______________________________________
		// if(avgVel> minAvgVel){
		if(avgRepelFrc >minAvgRepelFrc){
			var freq = avgVel.linlin(minAvgVel, maxAvgVel, 5, 40.0);
			var grainRate = avgVel.linlin(minAvgVel, maxAvgVel,1, 20.0);
			// var grainRate = avgRepelFrc.linlin(minAvgRepelFrc, maxAvgRepelFrc, 0.5, 2);
			~grainNote.gate_(1);
			~grainNote.freq_(freq)
			.grainRate_(grainRate).angle_(0.5pi)
			// .grainDur_(grainRate.reciprocal)
			.indx_(centroidY.linlin(minCentroidY, maxCentroidY, 0, 1.0))
			// .indx_(dist.linlin(minDist, maxDist, 0, 1.0))
			.theta_(centroidX.linlin(minCentroidX, maxCentroidX, 0.5pi, -0.5pi))
			.gain_(avgRepelFrc.linlin(minAvgRepelFrc, maxAvgRepelFrc, -6.0, 6.0))
			// .phi_(centroidY.linlin(minCentroidY, maxCentroidY, -pi, pi));
		} {
			~grainNote.gate_(0)
		};
	}, ~ofOscPath, nil, ~ofSendPort);

});
)



