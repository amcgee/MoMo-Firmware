var express = require('express'),
    app = express(),
    spawn = require('child_process').spawn;

function runMoMoCommand( args, out ) {
	momo = spawn('momo', args);
	output = ""
	momo.stdout.on('data', function (data) {
	  console.log('stdout: ' + data);
	  output += data;
	});

	momo.stderr.on('data', function (data) {
	  console.log('stderr: ' + data);
	  output += data;
	});

	momo.on('close', function (code) {
	  console.log('child process exited with code ' + code);
	  if ( code == 0 )
	  	out( null, output );
	  else
	  	out( code, output );
	});
}

app.get('/api', function( req, res ) {
	runMoMoCommand( ['help'], function( err, output ) {
		if ( !err )
			res.send( 200, "<pre>" + output + "</pre>" );
		else
			res.send( 500, "<pre>" + output + "</pre>" )
	})
})
app.get('/api/*', function( req, res ) {
	var args = req.path.split('/').slice(2);

	runMoMoCommand( args, function( err, output ) {
		if ( !err )
			res.send( 200, "<pre>" + output + "</pre>" );
		else
			res.send( 500, "<pre>" + output + "</pre>" )
	})
})

app.use(express.static(__dirname + '/static'));

app.listen( process.env.PORT || 3000 );
