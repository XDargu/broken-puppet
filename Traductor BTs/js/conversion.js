function translateNodeType(tree, type) {
	if (type == "Sequence")
		return "SEQUENCE";
	if (type == "Runner")
		return "ACTION";
	if (type == "Priority")
		return "PRIORITY";
	
	uiWarning("Tipo de nodo no identificado: " + type + ' en árbol: ' + tree.title);
	return "UNDEFINED";
}

function NodeWithTitleInActionsArray(array, node) {
	for (n in array) {
		if (array[n].title == node.title)
			return true;
	}
	return false;
}

function getActions(tree) {
	actions = [];
	for (node in tree.nodes) {
		if (tree.nodes[node].name == "Runner") {
			if (NodeWithTitleInActionsArray(actions, tree.nodes[node]) == false) {
				actions.push(tree.nodes[node]);
			}
		}	
	}
	
	return actions;
}

function NodeWithTitleInConditionsArray(array, node) {
	for (n in array) {
		if (array[n].condition.title == node.title)
			return true;
	}
	return false;
}

function getConditions(tree) {
	conditions = [];
	for (node in tree.nodes) {
		if (tree.nodes[node].name == "Condition") {
			if (NodeWithTitleInConditionsArray(conditions, tree.nodes[node]) == false) {
				conditions.push({ "node": tree.nodes[getChild(tree, node, 0)], "condition": tree.nodes[node]});
			}
		}
	}
	
	return conditions;
}

function getWarnings(tree) {
	for (node in tree.nodes) {
		// Warning if there is a non action node without children
		if (tree.nodes[node].children) {
			if ((tree.nodes[node].name != "Runner") && (tree.nodes[node].children.length == 0)) {
				uiWarning('Nodo de tipo ' + translateNodeType(tree, tree.nodes[node].name) + ' sin hijos: ' + tree.nodes[node].title + ' en árbol: ' + tree.title);
			}
		}
	}
}

function getParentNode(tree, node) {
	for (n in tree.nodes) {
		if (tree.nodes[n].children) {
			if (tree.nodes[n].children.indexOf(node) != -1) {
				// If the parent is a condition node, return the real parent
				if (tree.nodes[n].name == "Condition") {
					return getParentNode(tree, n);
				}
				else {
					return n;
				}
			}
		}
	}
	return undefined;
}

function getConditionNode(tree, node) {
	for (n in tree.nodes) {
		if (tree.nodes[n].children) {
			if (tree.nodes[n].children.indexOf(node) != -1) {
				// If the parent is a condition node, return it
				if (tree.nodes[n].name == "Condition") {
					return n;
				}
				else {
					return undefined;
				}
			}
		}
	}
	return undefined;
}

// Returns the Behaviour tree child (not a condition or a decorator)
function getChild(tree, node, index) {
	if (tree.nodes[tree.nodes[node].children[index]].name == "Condition")
		return getChild(tree, tree.nodes[node].children[index], 0);
	
	return tree.nodes[node].children[index];
}

function createCpp(input) {
	/* Referencia:
	
	#include "bt_soldier.h"

	void bt_soldier::create(string s)
	{
		name=s;
		createRoot("soldier", SEQUENCE, NULL, NULL);
		addChild("soldier", "escape", ACTION, (btcondition)&bt_soldier::conditionEscape, (btaction)&bt_soldier::actionEscape);
		addChild("soldier", "combat", SEQUENCE, NULL, NULL);
		addChild("soldier", "idle", ACTION, NULL, (btaction)&bt_soldier::actionIdle);
		addChild("combat", "pursuit", ACTION, NULL, (btaction)&bt_soldier::actionPursuit);
		addChild("combat","shoot",SEQUENCE,NULL,NULL);
		addChild("shoot", "shootgrenade", ACTION, NULL, (btaction)&bt_soldier::actionShootGrenade);
		addChild("shoot", "shootpistol", ACTION, NULL, (btaction)&bt_soldier::actionShootPistol);
	}

	int bt_soldier::actionIdle()
	{
		printf("%s: handling idle\n",name.c_str());
		return LEAVE;
	}

	int bt_soldier::actionPursuit()
	{
		printf("%s: handling pursuit\n",name.c_str());
		return LEAVE;
	}

	int bt_soldier::actionEscape()
	{
		printf("%s: handling escape\n",name.c_str());
		if (rand()%5) return LEAVE;
		return STAY;
	}

	bool bt_soldier::conditionEscape()
	{
		printf("%s: testing escape\n",name.c_str());
		return rand()%2;
	}



	int bt_soldier::actionShoot()
	{
		printf("%s: handling shoot\n",name.c_str());
		return LEAVE;
	}


	int bt_soldier::actionShootPistol()
	{
		printf("%s: handling shootpistol\n",name.c_str());
		return LEAVE;
	}


	int bt_soldier::actionShootGrenade()
	{
		printf("%s: handling shootgrenade\n",name.c_str());
		return LEAVE;
	}

	*/
	var tree = JSON.parse(input);
	var root = tree.root;
	
	var actions = getActions(tree);
	var conditions = getConditions(tree);
	
	var header = "";
	var title = tree.title;
	header += 
		  '#include "bt_' + title + '.h"\n'
		+ '\n'
		+ 'void bt_' + title + '::create(string s)\n'
		+ '{\n'
		+ '\tname = s;\n'
	;
	
	// Add root node
	header += '\tcreateRoot("' + tree.nodes[root].title + '", ' + translateNodeType(tree, tree.nodes[root].name) + ', NULL, NULL);\n';
	
	// Add childs, in preorder
	var currNode = root;
	var stack = [root];
	
	while (stack.length > 0) {
		
		currNode = stack.pop();
		
		var parNode = getParentNode(tree, currNode);
		var condNode = getConditionNode(tree, currNode);
		
		var nodeCondition = condNode ? '(btcondition)&bt_' + title + '::condition' + tree.nodes[condNode].title : 'NULL';
		var nodeAction = (tree.nodes[currNode].name == "Runner") ? '(btaction)&bt_' + title + '::action' + tree.nodes[currNode].title : 'NULL';
		
		if (parNode) {
			header += 
				'\taddChild("' + tree.nodes[parNode].title + '", "' + tree.nodes[currNode].title + '", ' + translateNodeType(tree, tree.nodes[currNode].name) + ', ' + nodeCondition +', ' + nodeAction + ');\n';
			;			
		}
		
		if (tree.nodes[currNode].children) {
			for (var i=tree.nodes[currNode].children.length-1; i>=0; i--) {
				stack.push(getChild(tree, currNode, i));
			}
		}
	}
	
	header += 
		  '}\n'
		+ '\n'
	;
	
	// Add action methods
	for (actionNode in actions) {
		header += 
			  '//' + actions[actionNode].description + '\n'
			+ 'int bt_' + title + '::action' + actions[actionNode].title + '()\n'
			+ '{\n'
			+ '\treturn LEAVE;\n'
			+ '}\n'
			+ '\n'
		;
	}
	
	// Add condition methods
	for (condNode in conditions) {
		header += 
			  '//' + conditions[condNode].condition.description + '\n'
			+ 'int bt_' + title + '::condition' + conditions[condNode].condition.title + '()\n'
			+ '{\n'
			+ '\treturn ' + conditions[condNode].condition.title + ';\n'
			+ '}\n'
			+ '\n'
		;
	}
	
	header += 
		  '};\n'
		+ '#endif'
	;
	
	return header;
}

function createH(input) {
	/* Referencia:
	
	#ifndef _BT_SOLDIER_INC
	#define _BT_SOLDIER_INC

	#include "bt.h"

	class bt_soldier:public bt
		{
		public:
			void create(string);

			int actionIdle();
			int actionEscape();
			int actionShoot();
			int actionShootGrenade();
			int actionShootPistol();
			int actionPursuit();
			bool conditionEscape();
		};

	#endif

	*/
	var tree = JSON.parse(input);
	var root = tree.root;
	
	var actions = getActions(tree);
	var conditions = getConditions(tree);
	getWarnings(tree);
	
	var header = "";
	var title = tree.title;
	header += 
		  '#ifndef _BT_SOLDIER_INC\n'
		+ '#define _BT_SOLDIER_INC\n'
		+ '\n'
		+ '#include "bt.h"\n'
		+ '\n'
		+ 'class bt_' + title + ':public bt\n'
		+ '{\n'
		+ 'public:\n'
		+ 'void create(string);\n'
		+ '\n'
	;
	
	// Add the actions
	for (actionNode in actions) {		
		header += 
			  '\t//' + actions[actionNode].description + '\n'
			+ '\tint action' + actions[actionNode].title + '();\n'
		;
	}
	
	header += '\n';
	
	// Add the conditions
	for (condNode in conditions) {		
		header += 
			  '\t//' + conditions[condNode].condition.description + '\n'
			+ '\tint condition' + conditions[condNode].condition.title + '();\n'
		;
	}
	
	header += 
		  '};\n'
		+ '\n'
		+ '#endif'
	;
	
	return header;
}

function getFileName(input) {
	var tree = JSON.parse(input);
	return tree.title;
}

function validateJSON(input, showError, theFile) {
  // Validate JSON
  var isValid = true;
  try {
	var mJson = JSON.parse(input);
	// First level Validation
	params = [
		{ name: "title", type: "string"}, 
		{ name: "description", type: "string"}, 
		{ name: "root", type: "string"},
		{ name: "properties", type: "object"},
		{ name: "nodes", type: "object"}
	];
	
	for (par in params) {
		if (mJson[params[par].name] == undefined) { if (showError) { uiError('JSON no válido, falta el parámetro ' + params[par].name); } isValid = false;}
		else if ((typeof mJson[params[par].name]) != params[par].type) {  if (showError) { uiError('JSON no válido, parámetro ' + params[par].name + ' debe ser de tipo "' + params[par].type + '", no de tipo "' + (typeof mJson[params[par].name]) + '"'); } isValid = false; }
	}
	
	// Node validation
	if (mJson.nodes) {
		nodeParams = [
			{ name: "id", type: "string"}, 
			{ name: "name", type: "string"}, 
			{ name: "title", type: "string"},
			{ name: "description", type: "string"},
			{ name: "parameters", type: "object"},
			{ name: "properties", type: "object"}
		];			
		
		for (node in mJson.nodes) {
			var objNode = mJson.nodes[node];
			if ((typeof mJson.nodes) != "object") { uiError('JSON no válido, el nodo ' + objNode + 'debe ser un objecto'); }
			for (par in nodeParams) {
				if (objNode[nodeParams[par].name] == undefined) {  if (showError) {  uiError('JSON no válido, al nodo ' + objNode + ' le falta el parámetro ' + nodeParams[par].name); } isValid = false; }
				else if ((typeof objNode[nodeParams[par].name]) != nodeParams[par].type) {  if (showError) {  uiError('JSON no válido, en el nodo ' + objNode + ' el parámetro ' + nodeParams[par].name + 'debe ser de tipo "' + nodeParams[par].type + '", no de tipo "' + (typeof objNode[nodeParams[par].name]) + '"'); } isValid = false; }
			}
		}
	}
  }
  catch (e) {
	   if (showError) { uiError('Error parseando el archivo "' + theFile.name + '": ' + e.message); }
	  isValid = false;
  }
  
  return isValid;
}

function json2cpp(input, index, callback) {
	
	var reader = new FileReader();

      // Closure to capture the file information.
      reader.onload = (function(theFile) {
        return function(e) {
		    var isValid = validateJSON(e.target.result, false, theFile);
		  
		  if (isValid)
			callback(createCpp(e.target.result), getFileName(e.target.result), index);
		  else
			callback(null, null, index);			
        };
      })(input);

      // Read in the image file as a data URL.
      reader.readAsText(input);
}

function json2h(input, index, callback) {
	
	var reader = new FileReader();

      // Closure to capture the file information.
      reader.onload = (function(theFile) {
        return function(e) {
		  var isValid = validateJSON(e.target.result, true, theFile);
		
		  if (isValid)
			callback(createH(e.target.result), getFileName(e.target.result), index);
		  else
			callback(null, null, index);
        };
      })(input);

      // Read in the image file as a data URL.
      reader.readAsText(input);
}

function uiError(stringError) {
	var htmlError = 
		  '<div class="ui-widget">'
		+ '<div class="ui-state-error ui-corner-all" style="padding: 0 .7em;">'
		+ '<p><span class="ui-icon ui-icon-alert" style="float: left; margin-right: .3em;"></span>'
		+ '<strong>Error:</strong>' + stringError + '</p>'
		+ '</div>'
		+ '</div>'
	;   

   $("#error_zone").append(htmlError);
}

function uiWarning(stringWarning) {
	var htmlError = 
		  '<div class="ui-widget">'
		+ '<div class="ui-state-highlight ui-corner-all" style="margin-top: 20px; padding: 0 .7em;">'
		+ '<p><span class="ui-icon ui-icon-info" style="float: left; margin-right: .3em;"></span>'
		+ '<strong>Aviso:</strong>' + stringWarning + '</p>'
		+ '</div>'
		+ '</div>'
	;   

   $("#error_zone").append(htmlError);
}