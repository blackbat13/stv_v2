#!/usr/bin/env node

/**
 * Minimal script to clasterise nodes in a .dot file
 * `arg1` - original dot file;
 * `arg2` - mapping file (optional, by default reads from mapping in *scc.txt*);
 */

/*
Example *scc.txt* content:
[ 17 20 22 23 ]
[ 7 10 15 18 ]
[ 8 11 16 19 ]
[ 1 2 6 9 ]
[ 21 ]
[ 12 ]
[ 13 ]
[ 3 ]
[ 14 ]
[ 4 ]
[ 5 ]
[ 0 ]
*/

const fs = require('fs');
const args = process.argv.slice(2);

if(!args?.length>=1){
    console.log("ERR: no source .dot file in the argument");
    return 0;
}

const dotSourceFile = args[0];
const mappingFile = args[1];

let dot = fs.readFileSync(dotSourceFile,'utf-8');
let str = fs.readFileSync(mappingFile, 'utf-8');

let res = [];
let dict = {};

str.split('\n').filter(x=>x).forEach((line,i) => {
    res.push(line.split(' ').filter(y=>y).filter(x=>x!='[' && x!=']'));
});

dot = dot.split('\n').filter(x=>x).map((line,ind)=>{
    let m;
    m = line.match(/(\d+)->(\d+)/) || line.match(/\d+(?=\[)/)
    if(m?.length==1){       // node
        dict[m[0]] = line;
        return "";
    }
    return line;
}).filter(x=>x)

let chunks = "";
let ctr = 0;
for( const comb of res){
    chunks += `subgraph cluster_${ctr++}{\nstyle=filled;\ncolor="#efefef";\n`;
    for(const val of comb){
        // console.log(val);
        chunks+=dict[val];
        chunks+="\n";
    }
    chunks+=`}`;
}
// insert chunks before the last line
dot.splice(dot.length-1,0,chunks);
dot = dot.join('\n')

fs.writeFileSync(`${dotSourceFile.split('.').slice(0, -1).join('.')}__clusterized.dot`,dot);