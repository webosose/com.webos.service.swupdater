import Button from '@enact/moonstone/Button';
import kind from '@enact/core/kind';
import PropTypes from 'prop-types';
import React from 'react';

import css from './Nav.less';

const Nav = kind({
    name: 'Nav',

    propTypes: {
        tabs: PropTypes.array.isRequired,
        onTabChange: PropTypes.func.isRequired,
        selectedTab: PropTypes.string.isRequired,
    },

    styles: {
        css,
        className: 'nav'
    },

    computed: {
        tabButtons: ({tabs, onTabChange, selectedTab}) => {
            return tabs.map((tab, index) => {
                return (
                    <Button
                        backgroundOpacity="translucent"
                        className={css.button}
                        key={index}
                        onClick={onTabChange}
                        selected={tab === selectedTab}
                    >
                        {tab}
                    </Button>
                );
            });
        },
    },

    render: ({tabButtons, ...rest}) => {
        delete rest.tabs;
        delete rest.onTabChange;
        delete rest.selectedTab;
        return (
            <div {...rest}>
                {tabButtons}
            </div>
        );
    }
});

export default Nav;
