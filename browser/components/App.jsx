import React from 'react';
import {render} from 'react-dom';

import Memory from 'components/Memory';
import M68k from 'components/M68k';
import Program from 'components/Program';

import 'styles/main.scss';

function saveRomInBrowser(data) {
    console.log('Saving to browser storage...');

    const string = new TextDecoder('utf8').decode(data);
    localStorage.setItem('rom', string);
    console.log(data, string)
    console.log(`File saved (${data.length} bytes)`);
}

function loadRomFromBrowser() {
    console.log('Checking browser storage...');

    const rom = localStorage.getItem('rom');
    if (rom) {
        const data = new TextEncoder('utf8').encode(rom);
        console.log(`File found (${data.length} bytes)`);
        return data;
    }
    else
        console.log('No file found');
}

function loadRom(data) {
    Module.HEAP8.set(data, Module.memory);
}

class App extends React.Component {

    render () {
        return (
            <div className="app">
                <input type="file" onChange={this.handleFile.bind(this)}/>
                <Memory rowCount={20} />
                <Program rowCount={20} />
                <M68k />
            </div>
        );
    }

    componentDidMount() {
        //const data = loadRomFromBrowser();
        //if (data)
        //    loadRom(data);
    }

    handleFile(event) {
        const reader = new FileReader();
        const file = event.target.files[0];

        reader.onload = load => {
            const data = load.target.result;

            //saveRomInBrowser(data);
            loadRom(data);
        };

        console.log(`Loading "${file.name}"...`);
        reader.readAsArrayBuffer(file);
    }
}

render(<App/>, document.querySelector('main'));
