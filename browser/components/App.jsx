import React from 'react';
import {render} from 'react-dom';

import Memory from 'components/Memory';
import Program from 'components/Program';

import 'styles/main.scss';

class App extends React.Component {
    render () {
        return (
            <div className="app">
                <Memory rowCount={20} />
                <div className="spacer"></div>
                <Program rowCount={20} />
            </div>
        );
    }
}

render(<App/>, document.querySelector('main'));
