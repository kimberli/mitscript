
function gramarify(text){
	rules = text.split(';;');
	out = "";
	for(var x in rules){
		out += "<tr><td>" + rules[x] + "</tr></td>";	
	}
	return '<table style="table"><tbody>'+out+'</tbody></table>';
}

function replacements(text){
	return text.replace(/-&gt;/g, "\\rightarrow");	
}

function rulify(text){
	topbot = text.split('---');
	toprule = replacements(topbot[0]);
	botrule = replacements(topbot[1]);
	out = "$\\frac{"+toprule+"}{"+botrule+"}$"; 
	out =  "<tr><td>" + out + "</tr></td>"
	console.log(out);
	return '<table style="table"><tbody>'+out+'</tbody></table>';
}

function doExample(name){
var text = document.getElementById(name).value;
eval(text);
}

function processTag(name, f){
	var x = document.getElementsByTagName(name);
	for(var i=0; i<x.length; ++i){
		x[i].innerHTML = f(x[i].innerHTML, x[i].id);
	}
}

function jsexamplify(text, name){
	var out = '<p><textarea id ="Example1" cols="80" rows="15">' +
	text +
    '</textarea>' +
    '<input type="button"  onClick="doExample(\\"'+name+'\\");" value="Run '+name+'"></p>'
	return out;
}

function processDocument(){
	processTag("grammar", gramarify);
	processTag("rule", rulify);
	processTag("jsexample", jsexamplify);
}