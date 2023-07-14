const fs = require('fs');
const path = require('path');
const util = require('util')

let arr = fs.readdirSync('.', {
        encoding:'utf-8',
        withFileTypes:true
    }).filter(entry=>{
        return entry.isDirectory()
    }).flatMap( (curr)=>(
        fs.readdirSync(
                path.join('.', curr.name),'utf-8'
            ).map(
                x=>path.join('.', curr.name, x)
            )
        )
    );

let obj = {};


for(const fname of arr){
    let lines = fs.readFileSync(fname,'utf-8').split('\n').filter(x=>x);
    
    let isMetaG = 0;
    let isMetaV = 0;

    let isG=0;
    let isGV=0;
    let isV=0;

    if(fname.match(/gen.txt$/)){
        isG=1;
    }
    if(fname.match(/gverif.txt$/)){
        isGV=1;
    }
    if(fname.match(/verif.txt$/)){
        isV=1;
    }

    if(isG || isGV || isV){
        lines.forEach(line=>{
            let pair = line.split(':');
            let key = pair[0].replace(/[\s]/g,'');
            let vals = pair[1].replace(/[\[\]]/g,'').split(/[\s]+/).filter(x=>x).map(x=>Number(x))
            if(!obj[key]){
                obj[key]={}
            }         

            if(isG){
                obj[key]["gen"] = vals;
            }else if(isGV){
                obj[key]["gverif"] = vals;
            }else if(isV){
                obj[key]["verif"] = vals;
            }
        })
    }

    if(fname.match(/metag.txt$/)){
        isMetaG=1
    }
    if(fname.match(/metav.txt$/)){
        isMetaV=1;
    }


    if(isMetaG || isMetaV){
        lines = lines.join('\n')
        if(isMetaV){
            lines=lines.replace(/Verification result: OK/g,'').replace(/Verification result: ERR/g,'')
        }

        lines = lines.replace(/([^\[\]\s]*):/g,'"$1":').replace(/\](?=[^\]])/g,'\],').replace(/\]/g,'\}').replace(/\[/g,'\{')
        // console.log((lines));
        // console.log('===============================');
        
        lines = lines.replace(/\{([\d,\s]+)\}/g, "[$1]")
        // console.log((lines));
        // console.log('==============================');
        
        lines = "{"+lines+"}"
        // console.log((lines));
        // console.log('==============================');
        // console.log(JSON.parse(lines));

        let temp = JSON.parse(lines);

        for(const key in temp){
            if(!obj[key]){
                obj[key]={}
            }   
            obj[key][isMetaG ? 'metag' : 'metav'] = temp[key]
        }
    }    

   
}

fs.writeFileSync('results.json', JSON.stringify(obj), 'utf-8')
