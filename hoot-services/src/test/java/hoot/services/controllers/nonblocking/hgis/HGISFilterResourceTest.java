/*
 * This file is part of Hootenanny.
 *
 * Hootenanny is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * The following copyright notices are generated automatically. If you
 * have a new notice to add, please use the format:
 * " * @copyright Copyright ..."
 * This will properly maintain the copyright information. DigitalGlobe
 * copyrights will be updated automatically.
 *
 * @copyright Copyright (C) 2016 DigitalGlobe (http://www.digitalglobe.com/)
 */
package hoot.services.controllers.nonblocking.hgis;

import javax.ws.rs.WebApplicationException;

import org.junit.Test;
import org.junit.experimental.categories.Category;

import hoot.services.UnitTest;


public class HGISFilterResourceTest {

    @Test
    @Category(UnitTest.class)
    public void TestFilterNonHgisPois() throws Exception {
/*
        HGISFilterResource real = new HGISFilterResource();
        HGISFilterResource spy = Mockito.spy(real);

        HGISResource.mapExists(Matchers.anyString());

        ArgumentCaptor<Command> argCaptor = ArgumentCaptor.forClass(Command.class);

        Mockito.doNothing().when((JobControllerBase) spy).processJob(Matchers.anyString(), argCaptor.capture());

        FilterNonHgisPoisRequest request = new FilterNonHgisPoisRequest();
        request.setSource("testSrc1");
        request.setOutput("out1");

        spy.filterNonHgisPois(request);

        List<Command> args = argCaptor.getAllValues();
        Command param = args.get(0);

        JSONParser parser = new JSONParser();
        JSONObject result = (JSONObject) parser.parse(param);

        assertEquals("custom/HGIS/RemoveNonHgisPois.sh", result.get("exec"));
        assertEquals("bash", result.get("exectype"));
        assertNotNull(result.get("params"));

        JSONArray arr = (JSONArray) result.get("params");

        String connStr = spy.generateDbMapParam("testSrc1");
        assertEquals(((JSONObject) arr.get(0)).get("SOURCE"), connStr);

        connStr = spy.generateDbMapParam("out1");
        assertEquals(((JSONObject) arr.get(1)).get("OUTPUT"), connStr);
*/
    }

    @Test(expected = WebApplicationException.class)
    @Category(UnitTest.class)
    public void TestInvalidSource() throws Exception {
        FilterNonHgisPoisRequest request = new FilterNonHgisPoisRequest();
        request.setSource(null);
        request.setOutput("out1");
        HGISFilterResource real = new HGISFilterResource();
        real.filterNonHgisPois(request);
    }

    @Test(expected = WebApplicationException.class)
    @Category(UnitTest.class)
    public void TestInvalidOutput() throws Exception {
        FilterNonHgisPoisRequest request = new FilterNonHgisPoisRequest();
        request.setSource("source");
        request.setOutput(null);
        HGISFilterResource real = new HGISFilterResource();
        real.filterNonHgisPois(request);
    }

    @Test(/*expected = WebApplicationException.class*/)
    @Category(UnitTest.class)
    public void TestInvalidNoMap() throws Exception {
/*
        HGISFilterResource real = new HGISFilterResource();
        HGISFilterResource spy = Mockito.spy(real);

        HGISResource.mapExists(Matchers.anyString());

        ArgumentCaptor<Command> argCaptor = ArgumentCaptor.forClass(Command.class);

        Mockito.doNothing().when(spy).processJob(Matchers.anyString(), argCaptor.capture());
        FilterNonHgisPoisRequest request = new FilterNonHgisPoisRequest();
        request.setSource("testSrc1");
        request.setOutput("out1");
        spy.filterNonHgisPois(request);
*/
    }
}
