import React from 'react';
import {render} from 'react-dom';

import Memory from 'components/Memory';
import M68k from 'components/M68k';
import Program from 'components/Program';

import 'styles/main.scss';

function saveRom(data) {

    // Store the ROM in browoser storage
    //var uint8array = new TextEncoder(encoding).encode(string);
    //var string = new TextDecoder(encoding).decode(uint8array);

    console.log('Saving to browser storage...');
    localStorage.setItem('rom', new TextDecoder('utf8').decode(data));
    console.log('File saved');
}

function loadRom() {
    console.log('Checking browser storage...');

    const rom = localStorage.getItem('rom');
    if (rom) {
        console.log('ROM file found');
        const data = new TextEncoder(encoding).encode(string);
        // TODO
    }
    else {
        console.log('No ROM file found');
    }
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
        // Try to load a ROM from the browser storage
        loadRom();
    }

    handleFile(event) {
        const reader = new FileReader();
        const file = event.target.files[0];

        reader.onload = load => {
            const data = load.target.result;

            // Store the ROM in the browser
            saveRom(data);

            //
        };

        console.log(`Loading "${file.name}"...`);
        reader.readAsArrayBuffer(file);
    }
}

render(<App/>, document.querySelector('main'));
