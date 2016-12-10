import React from 'react';
import {render} from 'react-dom';

import Debugger from 'components/Debugger';

import 'styles/main.scss';

class App extends React.Component {
    render () {
        return (
            <div>
                <Debugger rowCount={20} />
            </div>
        );
    }
}

render(<App/>, document.querySelector('main'));
