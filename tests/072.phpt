--TEST--
Check for Yaf_Route_Map::assemble
--SKIPIF--
<?php if (!extension_loaded("yaf")) print "skip"; ?>
--FILE--
<?php

$router = new Yaf_Router();

$route  = new Yaf_Route_Map();

$router->addRoute("map", $route);

var_dump($router->getRoute('map')->assemble(
                        array(
                                'controller' => 'foo_bar'
                        ),
                        array(
                                'tkey1' => 'tval1',
                                'tkey2' => 'tval2'
                             )
                        )
);

$route = new Yaf_Route_Map(true, '_');
$router->addRoute("map", $route);

var_dump($router->getRoute('map')->assemble(
                        array(
                                'action' => 'foo_bar'
                        ),
                        array(
                                'tkey1' => 'tval1',
                                'tkey2' => 'tval2'
                             )
                        )
);

--EXPECTF--
string(32) "/foo/bar?tkey1=tval1&tkey2=tval2"
string(34) "/foo/bar/_/tkey1/tval1/tkey2/tval2"